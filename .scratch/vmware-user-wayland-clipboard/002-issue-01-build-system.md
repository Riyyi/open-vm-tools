# Add `--with-wayland` configure flag and build support

**Status:** ready-for-agent

**Parent:** .scratch/vmware-user-wayland-clipboard/001-prd-wayland-clipboard-support.md

## What to build

Add `--with-wayland` to `configure.ac` with values `yes`/`no`/`auto`. When `yes` or `auto` (default), check for gtkmm-4.0 (`pkg-config --cflags --libs gtkmm-4.0` or equivalent). If `--with-wayland` is requested and gtkmm-4.0 is not found, exit configure with a clear error message. Update `dndcp/Makefile.am` to conditionally compile the Wayland source files (`copyPasteUIWayland.cpp`, `copyPasteDnDWayland.cpp` and their headers) only when the flag is enabled. The X11 build is unaffected when `--without-wayland`.

The Wayland source files themselves (empty stubs or real implementations) should be added to the build in this issue — the flag gates compilation, but the files exist from the start.

## Acceptance criteria

- [ ] `./configure --with-wayland` succeeds when gtkmm-4.0 is available
- [ ] `./configure --with-wayland` fails with a clear error when gtkmm-4.0 is not available
- [ ] `./configure --without-wayland` succeeds without gtkmm-4.0, building only the X11 path
- [ ] `dndcp/Makefile.am` compiles Wayland source files only when `--with-wayland` is set
- [ ] A skeleton `CopyPasteUIWayland` and `CopyPasteDnDWayland` (empty implementations) is added and compiles under the flag

## Blocked by

None - can start immediately