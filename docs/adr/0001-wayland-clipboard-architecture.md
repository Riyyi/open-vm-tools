# ADR-0001: Wayland Clipboard Support — Scope and Architecture

**Status:** accepted

## Context

Linux guests running VMware Tools in a Wayland session have no clipboard sharing. The existing `vmware-user` clipboard implementation (`CopyPasteUIX11`, `CopyPasteDnDX11`) is entirely X11-specific, relying on GTK3/GDK with X11 atoms, selection mechanisms (`GDK_SELECTION_CLIPBOARD`, `GDK_SELECTION_PRIMARY`), and ICCCM timestamp negotiation. On a Wayland compositor, these X11 calls either fail silently or crash the tools.

The `vmware-user-suid-wrapper` already detects Wayland sessions (`$XDG_SESSION_TYPE == "wayland"`) and passes a uinput file descriptor for DnD support.

## Decision

Implement Wayland-native clipboard support using GTK4, scoped to text-only (`CPFORMAT_TEXT`) over guestRPC v2+.

### Scope: text-only, guestRPC v2+

- **Include**: plain UTF-8 text clipboard (`CPFORMAT_TEXT`), guestRPC protocol path
- **Exclude**: RTF, PNG images, file lists (`CPFORMAT_FILELIST`), file contents (`CPFORMAT_FILECONTENTS`), legacy backdoor v1

DnD (drag-and-drop) is explicitly excluded — it requires virtual input device support (uinput) and `wl_data_device` protocol integration, which is a separate problem.

### Toolkit: GTK4 with Gdk::Clipboard

- Use `Gdk::Clipboard` (the Gio-based API in gtkmm-4.0) for clipboard read/write
- Use `Gdk::ContentProvider` for H→G clipboard (lazy data supply when compositor requests it)
- Use `Gdk::Clipboard::read_text_async()` callbacks for G→H clipboard
- Do not use `gdk_set_allowed_backends()` on Wayland — GTK auto-detects
- Remove the existing `gdk_set_allowed_backends("x11")` constraint only for the Wayland impl

### Code organization: parallel platform implementations

Follow the existing pattern where `CopyPasteDnDImpl` is the abstract interface and `CopyPasteDnDX11` / `CopyPasteDnDWayland` are concrete implementations:

```
CopyPasteDnDImpl          (abstract base)
  CopyPasteDnDX11         (X11: GTK3 + GDK/X11, all formats)
  CopyPasteDnDWayland     (Wayland: GTK4 + Gdk::Clipboard, text only)
```

New files:
- `dndcp/copyPasteUIWayland.{h,cpp}` — clipboard UI for Wayland
- `dndcp/dndGuestBase/copyPasteDnDWayland.{h,cpp}` — platform init for Wayland

The protocol layer (`GuestCopyPasteMgr`, `GuestDnDCPMgr`) is unchanged.

### Build system: `--with-wayland` configure flag

- Add `--with-wayland` to `configure.ac` with values `yes`/`no`/`auto`
- When enabled, check for GTK4 / gtkmm-4.0; fail with a clear error if not found
- `Makefile.am` conditionally compiles Wayland source files
- The X11 build remains unaffected when `--without-wayland`

### Runtime detection: compile-time gate + runtime session type

- The `--with-wayland` configure flag gates compilation of the Wayland impl
- At runtime, `CopyPasteDnDWrapper::Init` instantiates `CopyPasteDnDWayland` on Wayland sessions, `CopyPasteDnDX11` on X11 sessions
- Session type detection: `$XDG_SESSION_TYPE` env var (already detected by the suid wrapper) or `gdk_display_get_display_type()` at runtime

### No uinput required for clipboard

Clipboard (copy/paste) is pure data transfer over guestRPC — no input device events needed. The `--uinputFd` passed by the wrapper is for DnD only. No changes to the wrapper are needed for clipboard support.

## Consequences

- Wayland clipboard works on GNOME Shell, KDE Plasma, Sway, and other Wayland compositors via GTK4's abstraction
- X11 clipboard support remains unchanged and fully functional
- File transfer clipboard, RTF, and images remain X11-only for the foreseeable future
- Distributions can build with or without Wayland support via the configure flag
- The DnD plugin continues to use vmblock for file transfers (unchanged)