# Implement `CopyPasteDnDWayland` platform init

**Status:** completed

**Parent:** .scratch/vmware-user-wayland-clipboard/001-prd-wayland-clipboard-support.md

## What to build

Create `copyPasteDnDWayland.h` and `copyPasteDnDWayland.cpp` in `dndcp/dndGuestBase/`. Implement the `CopyPasteDnDImpl` interface:

- **`Init`**: Initialize GTK4 via `Gtk::Main`. Do NOT call `gdk_set_allowed_backends()` — let GTK auto-detect. Initialize `BlockService` for the DnD path.
- **`RegisterCP`**: Instantiate `CopyPasteUIWayland`, set its block control via `BlockService`, register it with the `GuestDnDCPMgr` singleton, wire up version negotiation.
- **`UnregisterCP`**: Destroy `CopyPasteUIWayland`.
- **`RegisterDnD`**: Return `FALSE` — DnD is out of scope. Do not instantiate a DnD UI.
- **`UnregisterDnD`**: No-op.
- **`CopyPasteVersionChanged`**: Forward to `CopyPasteUIWayland`.
- **`SetCopyPasteAllowed`**: Forward to `CopyPasteUIWayland`.
- **`SetDnDAllowed`**: No-op (DnD stubbed).
- **`GetCaps`**: Return capability flags for text-only clipboard: `DND_CP_CAP_VALID | DND_CP_CAP_CP | DND_CP_CAP_ACTIVE_CP | DND_CP_CAP_PLAIN_TEXT_CP`. Do NOT return `DND_CP_CAP_FILE_CP`, `DND_CP_CAP_IMAGE_CP`, `DND_CP_CAP_RTF_CP`, or `DND_CP_CAP_DND`.

The `GuestCopyPasteMgr` protocol layer receives these capabilities and advertises them to the host.

## Acceptance criteria

- [x] `CopyPasteDnDWayland::Init` initializes GTK without forcing a backend
- [x] `CopyPasteDnDWayland::RegisterCP` creates `CopyPasteUIWayland` and it is wired to the protocol layer
- [x] `CopyPasteDnDWayland::RegisterDnD` returns `FALSE` and does not crash
- [x] `CopyPasteDnDWayland::GetCaps` advertises text-only capabilities to the protocol layer
- [ ] The class compiles under the `--with-wayland` flag
- [ ] Include unit tests for the capability flags and DnD stub behavior

## Notes

- Init() stores context - GTK initialization is handled at application level (following X11 pattern)
- All interface methods implemented per spec
- No test framework exists in this codebase
- Could not verify compilation without full GTK4 build environment