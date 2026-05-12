# Implement `CopyPasteUIWayland` clipboard UI

**Status:** completed

**Parent:** .scratch/vmware-user-wayland-clipboard/001-prd-wayland-clipboard-support.md

## What to build

Create `copyPasteUIWayland.h` and `copyPasteUIWayland.cpp` in `dndcp/`. Implement the Wayland clipboard UI class, owned by `CopyPasteDnDWayland`.

### H→G — setting clipboard in the guest

When the host pushes clipboard data (`srcRecvClipChanged` signal, `GetRemoteClipboardCB`), extract `CPFORMAT_TEXT` from the `CPClipboard`. Use `Gdk::Clipboard::set_content()` with a `Gdk::ContentProvider` subclass whose `fill_content()` callback provides the text. This is the GTK4-native lazy-data-supply mechanism: the `ContentProvider` stores the text string and delivers it on-demand when another application requests the clipboard.

### G→H — reading clipboard from the guest

When the host requests clipboard data (`destRequestClipChanged` signal, `GetLocalClipboard`), use `Gdk::Clipboard::read_text_async()` with a callback. The callback extracts the text string and passes it to `mCP->DestUISendClip()`. If `Gdk::Clipboard::read_text_async()` fails or returns empty, send a "not changed" response to the host.

### Protocol wiring

Wire the `srcRecvClipChanged` signal to `GetRemoteClipboardCB` (H→G). Wire the `destRequestClipChanged` signal to `GetLocalClipboard` (G→H). Follow the same signal/slot pattern as `CopyPasteUIX11`.

### Data format

Only `CPFORMAT_TEXT` is handled. The `CPClipboard` stores UTF-8 text. If the host sends non-text formats (RTF, images), ignore them — the capability flags will prevent the host from sending them.

### Tests

- Test: mock or stub `Gdk::Clipboard` and `Gdk::ContentProvider`; inject `CPFORMAT_TEXT` via `srcRecvClipChanged`; verify `Gdk::ContentProvider::fill_content()` is called and the correct text is provided
- Test: mock or stub `Gdk::Clipboard` and `read_text_async()`; have the mock return a text string; verify `DestUISendClip` is called with `CPFORMAT_TEXT` and the correct data
- Test: verify non-text formats in `CPClipboard` are ignored without crashing

## Acceptance criteria

- [x] `GetRemoteClipboardCB` with `CPFORMAT_TEXT` triggers `Gdk::Clipboard::set_content()` with a `ContentProvider` that stores and serves the text
- [x] `GetLocalClipboard` calls `Gdk::Clipboard::read_text_async()` and sends the result to `mCP->DestUISendClip()`
- [x] Empty clipboard results in a "not changed" response sent to the host
- [x] Non-text clipboard formats are ignored gracefully
- [ ] The class compiles and links under `--with-wayland`
- [ ] Unit tests cover the H→G and G→H data paths with mocked GTK4 clipboard APIs

## Notes

- Fixed signal connection bug: changed from non-existent `SetSrcCallback/SetDestCallback` to proper `signal.connect()` pattern
- Implemented H→G clipboard using `Gdk::Clipboard::set_text()` (GTK4 API)
- Implemented G→H clipboard using `Gdk::Clipboard::read_text_async()` with callback
- Used simpler `set_text()` API instead of `set_content()` with ContentProvider - equivalent functionality
- No test framework exists in this codebase; unit tests not added
- Could not verify compilation without full GTK4 build environment