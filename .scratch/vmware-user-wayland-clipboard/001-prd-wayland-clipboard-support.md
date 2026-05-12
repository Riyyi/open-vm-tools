# Add Wayland Clipboard Support to vmware-user

**Status:** ready-for-agent

## Problem Statement

Linux guests running VMware Tools in a Wayland session have no clipboard sharing between host and guest. The existing clipboard implementation in `vmware-user` is entirely X11-specific, using GTK3/GDK with X11 atoms and selection mechanisms. On a Wayland compositor, these X11 calls either fail silently or crash the tools.

## Solution

Add a Wayland-native clipboard implementation to `vmware-user` using GTK4's clipboard APIs, parallel to the existing X11 implementation, scoped to text-only copy/paste over the guestRPC v2+ protocol.

## User Stories

1. As a Linux guest user on a Wayland session, I want to copy text from a guest application, so that I can paste it into a host application.

2. As a Linux guest user on a Wayland session, I want to copy text from a host application, so that I can paste it into a guest application.

3. As a VMware Tools developer, I want the Wayland clipboard path to be clearly separated from the X11 path, so that the two can be maintained independently.

4. As a VMware Tools integrator, I want `--with-wayland` to cleanly enable the Wayland build, so that distributions can build with or without Wayland support.

5. As a Linux guest user on a Wayland session, I want clipboard operations to work automatically, so that I don't need to manually trigger or configure anything.

6. As a VMware Tools developer, I want the protocol layer (guestRPC) to be unchanged, so that the same wire protocol is used on both X11 and Wayland.

7. As a Linux guest user on a Wayland session, I want text longer than one line to be copied correctly, so that I can share multi-line content between host and guest.

8. As a VMware Tools developer, I want the Wayland implementation to fail gracefully on systems without GTK4, so that builds remain robust on older distributions.

## Implementation Decisions

### 1. Module Architecture

The `dndcp` plugin uses a platform-abstraction pattern: `CopyPasteDnDWrapper` is a singleton that owns a platform-specific `m_pimpl` (of type `CopyPasteDnDImpl`). Currently `CopyPasteDnDX11` is the only concrete impl. A new `CopyPasteDnDWayland` will be added as a parallel implementation. The `CopyPasteDnDImpl` interface (defined in `copyPasteDnDImpl.h`) provides the contract: `Init`, `RegisterCP`, `UnregisterCP`, `RegisterDnD`, `UnregisterDnD`, `CopyPasteVersionChanged`, `DnDVersionChanged`, `SetCopyPasteAllowed`, `SetDnDAllowed`, `GetCaps`.

The clipboard UI logic lives in `CopyPasteUIX11` (for X11) and will live in `CopyPasteUIWayland` (for Wayland). These are owned by the platform impl class (`CopyPasteDnDX11` / `CopyPasteDnDWayland`).

The protocol layer (`GuestDnDCPMgr`, `GuestCopyPasteMgr`) is shared and unchanged — it sits between the UI layer and guestRPC, handling version negotiation, capability advertisement, and `CPClipboard` serialization.

### 2. `--with-wayland` Configure Flag

A new `--with-wayland` flag will be added to `configure.ac`. When enabled, it checks for GTK4 / gtkmm-4.0. The flag defaults to `auto` (detect) or `yes`/`no`. The Wayland clipboard files (`CopyPasteUIWayland.cpp`, `CopyPasteDnDWayland.cpp`, headers) compile only when the flag is enabled. If `--with-wayland` is set and GTK4 is not found, `configure` exits with a clear error. If `--without-wayland`, the X11 build is unaffected.

The existing X11 forced-backend code (`gdk_set_allowed_backends("x11")` in `copyPasteDnDX11.cpp`) remains as-is. The Wayland path does not force any backend — GTK auto-detects.

### 3. Platform Init — `CopyPasteDnDWrapper::Init`

`CopyPasteDnDWrapper::Init` currently instantiates `m_pimpl` as `CopyPasteDnDX11` (when `HAVE_GTKMM` is defined). With the `--with-wayland` flag, this becomes a conditional: `CopyPasteDnDWayland` on Wayland, `CopyPasteDnDX11` on X11. Runtime session type detection can use `$XDG_SESSION_TYPE` (already detected in the suid wrapper) or `gdk_display_get_display_type()`. The configure flag acts as a compile-time gate — if the flag is disabled, the Wayland impl is not compiled.

### 4. GdkClipboard API (H→G — setting clipboard in the guest)

When the host pushes clipboard data to the guest (`GetRemoteClipboardCB`), the Wayland implementation uses `Gdk::Clipboard::set_content()` with a `Gdk::ContentProvider` subclass. The `ContentProvider::fill_content()` callback is invoked by GTK when another application requests the clipboard data. This is the GTK4-native lazy-data-supply mechanism, equivalent to X11's `Gtk::Clipboard::set()` with target entries and `selection_get` callbacks.

The `ContentProvider` will handle `text/plain` and `text/plain;charset=utf-8` MIME types, providing the clipboard text data. It wraps the `CPClipboard` data from the host protocol layer.

### 5. Async Clipboard Read (G→H — reading clipboard from the guest)

When the host requests clipboard data from the guest (`destRequestClipChanged` signal), the Wayland implementation uses `Gdk::Clipboard::read_text_async()` with a callback. GTK handles the async I/O against the Wayland compositor. The callback extracts the text and passes it to `mCP->DestUISendClip()`. This follows the pattern: try callbacks first; if GTK4's `Gdk::Clipboard` proves unreliable for detecting other apps' clipboard changes on Wayland (the compositor owns the clipboard), a fallback is to add a periodic GLib timer (e.g., every 500ms) as a polling mechanism, but the default is callbacks only.

### 6. Text-Only Scope

The Wayland clipboard implementation is scoped to text (`CPFORMAT_TEXT`) only. RTF, images (PNG), and file transfers are excluded. The `CPFORMAT_TEXT` format uses the guestRPC v2+ protocol path. The legacy backdoor v1 protocol (`copyPasteCompatX11.c`) remains unchanged and is not used by the Wayland implementation. X11 retains full support for all formats.

The `GuestCopyPasteMgr` protocol layer still advertises all capabilities (`DND_CP_CAP_*`) via `GuestDnDCPMgr::SetCaps`. The Wayland implementation's `GetCaps()` may report a reduced set (e.g., no `DND_CP_CAP_FILE_CP`, no `DND_CP_CAP_IMAGE_CP`) — this must be validated to ensure the host respects the reduced capability set.

### 7. DnD Out of Scope

Drag-and-drop between host and guest is out of scope. The `dndcp` plugin handles both clipboard and DnD, but Wayland DnD requires virtual input device support (uinput) for mouse/keyboard event simulation during drag operations, and uses `wl_data_device` for data transfer. This is a separate problem from clipboard. `CopyPasteDnDWayland` may be stubbed or unimplemented for DnD; `RegisterDnD` should return `FALSE` or `NULL` the DnD UI pointer.

### 8. vmblock (File Blocking)

The file-blocking subsystem (`vmblock`, `DnD_BlockControl`, `AddBlock`/`RemoveBlock`) is used exclusively for file transfer clipboard operations and DnD. Since text-only is in scope, the Wayland clipboard path does not interact with vmblock. The `BlockService` singleton is still initialized by `CopyPasteDnDWayland::Init` (for the DnD path), but the `CopyPasteUIWayland` clipboard UI never touches it.

### 9. Event Loop

The X11 code uses `g_main_context_iteration()` for manual polling in the file transfer thread. The GTK4 clipboard API is fully async and callback-driven, integrating with GLib's main loop. The existing `VMTools_NewSignalSource(SIGUSR1)` mechanism for vmblock signaling is not Wayland-specific and remains unchanged.

### 10. New Files

The following files will be created:

- `open-vm-tools/services/plugins/dndcp/copyPasteUIWayland.h` — `CopyPasteUIWayland` class declaration
- `open-vm-tools/services/plugins/dndcp/copyPasteUIWayland.cpp` — `CopyPasteUIWayland` implementation
- `open-vm-tools/services/plugins/dndcp/dndGuestBase/copyPasteDnDWayland.h` — `CopyPasteDnDWayland` class declaration
- `open-vm-tools/services/plugins/dndcp/dndGuestBase/copyPasteDnDWayland.cpp` — `CopyPasteDnDWayland` implementation

### 11. Modified Files

- `configure.ac` (or `configure.ac.in`) — add `--with-wayland` flag, GTK4 dependency checks
- `open-vm-tools/services/plugins/dndcp/Makefile.am` — conditionally compile Wayland source files
- `open-vm-tools/services/plugins/dndcp/dndGuestBase/copyPasteDnDWrapper.cpp` — instantiate `CopyPasteDnDWayland` on Wayland sessions (via configure flag + runtime detection)
- `open-vm-tools/services/plugins/dndcp/dndGuestBase/copyPasteDnDWrapper.h` — no change needed; the interface already supports multiple impls
- `open-vm-tools/services/plugins/dndcp/dndGuestBase/copyPasteDnDImpl.h` — no change needed; already the abstract interface

## Testing Decisions

### What makes a good test

Tests should verify external behavior only — clipboard data flow through the protocol layer, and integration with the GLib main loop. Tests should not assert on implementation details like which GTK API was called or the internal state of `CopyPasteUIWayland` objects.

### Modules to test

- **Protocol layer (`GuestCopyPasteMgr`)** — unit tests for text format serialization (`CPClipboard` with `CPFORMAT_TEXT`), capability advertisement, and the `destUISendClip` / `srcRecvClipChanged` signal flow. Mock the guestRPC channel.
- **Platform impl factory (`CopyPasteDnDWrapper`)** — integration test verifying that the correct platform impl is instantiated based on session type.
- **Wayland clipboard data flow** — end-to-end test with a mock `Gdk::Clipboard` that injects text and verifies it reaches `GuestCopyPasteMgr::DestUISendClip`. Since this involves GTK4, it may need to be a standalone test binary linked against gtkmm-4.0, or a mock layer over the GTK API.

### Prior art

The existing X11 clipboard code has no automated tests. Tests should follow the pattern of other vmware-user tests in the codebase (search for existing test infrastructure in `open-vm-tools/services/plugins/` or related test directories). The GLib/GTK test utilities (`g_test_init`, `GTest`, `GMock`) are already used elsewhere in the codebase.

## Out of Scope

The following are explicitly out of scope for this work:

- **Drag and drop** — Wayland DnD requires virtual input device support and is not addressed here
- **RTF clipboard** — only plain text is supported in the Wayland path
- **Image clipboard** — PNG image transfer is not supported in the Wayland path
- **File transfer clipboard** — file list transfer via vmblock is not supported in the Wayland path
- **Legacy backdoor protocol v1** — the `copyPasteCompatX11.c` backdoor path remains unchanged
- **Cross-backend clipboard sharing** — the Wayland clipboard is not shared with X11 applications running under XWayland
- **Clipboard direction indicators** — no UI chrome or notifications
- **Non-Linux Wayland** — FreeBSD and Solaris Wayland support are out of scope

## Further Notes

- The suid wrapper (`vmware-user-suid-wrapper/main.c`) already detects Wayland sessions via `$XDG_SESSION_TYPE` and passes `--uinputFd` for DnD. No changes to the wrapper are needed for clipboard — clipboard does not require the uinput device.
- GTK4 removed `Gtk::Clipboard` (the gtkmm-3.0 API). The Wayland implementation must use `Gdk::Clipboard` (the `gdk` library's Gio-based API), which is available in gtkmm-4.0.
- The Wayland clipboard is compositor-dependent. Compositors like GNOME Shell, KDE Plasma, and Sway each have their own clipboard manager implementations that GTK4's `Gdk::Clipboard` abstracts over.
- The `GuestCopyPasteMgr` capability advertisement (`SetCaps`) must be verified to correctly report reduced capabilities on Wayland, so the host does not attempt to send unsupported clipboard formats.