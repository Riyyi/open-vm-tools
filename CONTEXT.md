# Context — open-vm-tools

## Domain

**vmware-user** is the Linux guest userspace daemon that provides VMware Tools integration (clipboard sharing, drag-and-drop, pointer synchronization) between a Linux guest VM and the VMware hypervisor.

**vmware-user-suid-wrapper** is a setuid-root helper that manages the vmblock filesystem and passes file descriptors to `vmware-user`.

**DnD/CP** (Drag-and-Drop / Copy-Paste) is the plugin module (`dndcp`) that handles clipboard sharing and drag-and-drop between guest and host.

## Key Components

### Protocol stack

- **guestRPC** — bi-directional RPC channel between guest tools and the VMX (host VMware process) over a virtual serial port or vsock
- **CopyPaste** / **GuestCopyPasteMgr** — manages clipboard state and data flow, handles version negotiation
- **GuestDnDCPMgr** — singleton orchestrator for both DnD and Copy/Paste, owns `GuestCopyPasteMgr`

### Clipboard formats

- `CPFORMAT_TEXT` — plain UTF-8 text
- `CPFORMAT_RTF` — Rich Text Format
- `CPFORMAT_IMG_PNG` — PNG image
- `CPFORMAT_FILELIST` — file list with relative paths
- `CPFORMAT_FILECONTENTS` — file contents with XDR encoding

### Protocol versions

- **v1** — uses VMware backdoor I/O port, text-only, phase-based (Ungrab = get from guest, Grab = push to guest)
- **v2+** — uses guestRPC channel, supports all clipboard formats, bidirectional

### Platform abstraction

- `CopyPasteDnDImpl` — abstract base for platform-specific DnD/CP implementations
- `CopyPasteDnDX11` — X11 implementation (GTK3, GDK/X11, X11 atoms/selections)
- `CopyPasteUIWayland` — Wayland implementation (GTK4, Gdk::Clipboard) [planned]
- `CopyPasteDnDWrapper` — singleton that owns the platform impl, provides init/registration/reset logic

### Infrastructure

- **vmblock** — kernel module + userspace fuse mount that blocks access to files during transfer, allowing the tools to intercept and redirect
- **DnDBlockControl** — API for adding/removing vmblock entries
- **BlockService** — singleton managing vmblock initialization and SIGUSR1 shutdown handler

## Glossary

| Term | Meaning |
|------|---------|
| G→H | Guest-to-host clipboard (copy from guest app, paste into host app) |
| H→G | Host-to-guest clipboard (copy from host app, paste into guest app) |
| guestRPC | RPC channel over virtual serial port / vsock to VMX |
| backdoor | VMware hypervisor I/O port-based communication (v1 only) |
| FCP | File Copy Paste — file transfer over the clipboard protocol |
| DnD | Drag-and-drop — drag files between host and guest |
| vmblock | Kernel blocking driver for secure file transfer |
| staging dir | Temp directory where incoming files are written before being moved to target |
| suid wrapper | `vmware-user-suid-wrapper` — setuid helper that opens vmblock/uinput FDs |

## Architecture notes

The DnD/CP plugin (`dndcp`) is loaded by `vmware-user` as a shared plugin. The plugin framework is in `open-vm-tools/lib/` and is shared across all plugins.

Clipboard and DnD share the same plugin and `CopyPasteDnDWrapper` singleton, but have separate UI classes (`CopyPasteUIX11` vs `DnDUIX11`).