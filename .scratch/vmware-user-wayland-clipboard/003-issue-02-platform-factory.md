# Conditionally instantiate Wayland platform impl

**Status:** ready-for-agent

**Parent:** .scratch/vmware-user-wayland-clipboard/001-prd-wayland-clipboard-support.md

## What to build

Modify `CopyPasteDnDWrapper::Init` in `copyPasteDnDWrapper.cpp` to detect the session type at runtime and instantiate the correct platform implementation. On Wayland sessions, instantiate `CopyPasteDnDWayland`; on X11 sessions, `CopyPasteDnDX11`. Use `gdk_display_get_display_type()` (returning `GDK_DISPLAY_TYPE_WAYLAND`) or the `$XDG_SESSION_TYPE` environment variable as the detection mechanism. The `--with-wayland` configure flag acts as a compile-time gate — the Wayland impl is only compiled when the flag is set, so `#ifdef` or `#if HAVE_GTKMM`-guard the instantiation.

This does not change the `CopyPasteDnDImpl` interface — `copyPasteDnDWrapper.h` and `copyPasteDnDImpl.h` are unchanged.

## Acceptance criteria

- [ ] `CopyPasteDnDWrapper::Init` detects a Wayland session and instantiates `CopyPasteDnDWayland`
- [ ] `CopyPasteDnDWrapper::Init` detects an X11 session and instantiates `CopyPasteDnDX11`
- [ ] The conditional compilation is guarded by the `--with-wayland` configure result
- [ ] When `--without-wayland`, `CopyPasteDnDWayland` is not referenced and the X11 path builds unchanged

## Blocked by

- .scratch/vmware-user-wayland-clipboard/002-issue-01-build-system.md