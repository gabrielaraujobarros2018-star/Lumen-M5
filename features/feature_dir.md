# Directory Structure Conventions for Adding New Features in a Custom Linux Distribution

When building or extending a **full custom Linux distribution** (e.g. from scratch like Linux From Scratch, or using tools like Yocto, Buildroot, T2SDE, or your own build system), deciding **where** to place new files and features is guided by the **Filesystem Hierarchy Standard (FHS)**, real-world practices from major distributions (Debian, Fedora, Arch, Alpine, Gentoo, Void, etc.), and practical considerations for boot-critical paths, package management, and maintainability.

This document summarizes the most common and recommended locations in 2025–2026-era distro development.

## Quick Decision Tree

- **Feels like part of the core/base operating system?**  
  → Use `/usr/` hierarchy (almost everything goes here in modern custom distros)

- **Large, mostly self-contained application or suite?** (custom DE fork, browser engine, office suite, game engine, Electron app, etc.)  
  → Use `/opt/yourdistro-name/` or `/opt/vendor-appname/`

- **Needs to be available very early in boot (before `/usr` is mounted)?**  
  → `/bin`, `/sbin`, `/lib`, `/lib64` (or symlinks → `/usr/...` on systemd-based systems)

- **System-wide configuration?**  
  → `/etc/` (or `/etc/yourfeature.d/` drop-in style)

- **Variable/runtime data (logs, databases, caches, state)?**  
  → `/var/`

- **Just experimenting / local development builds (not for final image)?**  
  → `/usr/local/`

## Recommended Locations Table

| Feature Type                                      | Primary Location(s)                              | Secondary / Alternatives                     | Real-World Examples                                      | Rationale / Notes                                                                 |
|---------------------------------------------------|--------------------------------------------------|----------------------------------------------|----------------------------------------------------------|-----------------------------------------------------------------------------------|
| Core utilities, daemons, libraries, headers       | `/usr/bin`, `/usr/sbin`, `/usr/lib`, `/usr/libexec`, `/usr/include`, `/usr/share` | `/bin`, `/sbin`, `/lib` (symlinks)           | `bash`, `coreutils`, `systemd`, `glibc`, `openssl`       | Main read-only hierarchy owned by the distro. Most new features belong here.      |
| Very early-boot critical binaries                 | `/bin`, `/sbin`, `/lib`, `/lib64`                | Symlink to `/usr/...`                        | `busybox`, `init`, `e2fsck`, `udevd`, `modprobe`         | Required in initramfs / single-user / recovery mode before `/usr` mount           |
| System-wide configuration files                   | `/etc`, `/etc/<feature>/`, `/etc/<feature>.d/`   | `/usr/share/defaults/` (fallback defaults)   | `/etc/fstab`, `/etc/systemd/`, `/etc/ssh/sshd_config`    | Configs only — never put binaries or large data here                              |
| Desktop environments, window managers, themes     | `/usr/share/<de>/`, `/usr/share/themes/`, `/usr/share/icons/`, `/usr/share/applications/` | `/usr/share/xsessions/`, `/usr/share/wayland-sessions/` | GNOME, KDE Plasma, XFCE, Sway, Hyprland                  | Follows XDG/freedesktop.org specifications                                        |
| Large self-contained / add-on applications        | `/opt/<yourdistro>-<app>`, `/opt/<vendor>-<app>` | `/usr/lib/<app>/` (sometimes)                | `/opt/google/chrome`, `/opt/firefox`, `/opt/blender`     | FHS reserves `/opt` for independent packages; easy to replace or remove           |
| Locally built / experimental / not-yet-packaged   | `/usr/local/bin`, `/usr/local/lib`, `/usr/local/share` | —                                            | Hand-compiled nginx, custom git, personal scripts        | For end-user machines — avoid in final distro images                              |
| Kernel modules, firmware, DKMS-style drivers      | `/lib/modules/$(uname -r)/`, `/usr/lib/firmware/` | `/lib/firmware/`                             | NVIDIA/AMD proprietary modules, Wi-Fi/Bluetooth firmware | Follows kernel and initramfs expectations                                         |
| Variable / per-machine data                       | `/var/lib/<feature>/`, `/var/log/<feature>/`, `/var/cache/<feature>/` | `/var/spool/`, `/var/run/` (ephemeral)       | `/var/lib/dpkg/`, `/var/log/journal/`, `/var/cache/apt/` | Never put read-only files or executables here                                     |
| Man pages, documentation                          | `/usr/share/man/`, `/usr/share/doc/<package>/`   | `/usr/local/share/man/` (local)              | Every package’s man pages and READMEs                    | Standard location; compress with gzip/xz                                          |
| Locale / i18n files                               | `/usr/share/locale/`, `/usr/share/i18n/`         | —                                            | gettext translations                                     | Follow gettext conventions                                                        |
| Fonts                                             | `/usr/share/fonts/`, `/usr/local/share/fonts/`   | `/usr/share/fonts/truetype/`, etc.           | Google Fonts, nerd-fonts, system fonts                   | Fontconfig expects this structure                                                 |

## Modern Trends (2025–2026)

- Most distributions now symlink `/bin` → `/usr/bin`, `/sbin` → `/usr/sbin`, `/lib` → `/usr/lib`, etc.  
  → You can usually just write files directly into `/usr/...` and let the symlinks handle early boot compatibility.

- `/usr/merge` (merged `/usr`) is now effectively universal among new/minimal distros.

- `/opt` is increasingly popular again for anything that wants strong versioning, easy replacement, or isolation (especially proprietary or very large runtimes).

- Avoid scattering files across many top-level directories — stick to FHS as much as possible for user/admin sanity.

## Summary – Where Most New Features Go in Practice

New feature type                            → Recommended location
───────────────────────────────────────────────────────────────────────────────
Core system tools, services, libs           → /usr/bin /usr/lib /usr/share …
Configuration                               → /etc /etc/yourfeature/
Big standalone apps / suites                → /opt/yourdistro-appname/
Early boot necessities                      → /bin /sbin /lib (or symlinks)
Variable data (logs, state, caches)         → /var/lib/yourfeature/ /var/log/
Desktop/WM themes, icons, .desktop files    → /usr/share/themes /usr/share/icons /usr/share/applications
Experimental / dev-only builds              → /usr/local/ (temporary!)
Kernel-related (modules, firmware)          → /lib/modules/ /usr/lib/firmware/

Bottom line for most custom distro developers today:
Put almost everything new under /usr/.
Use /opt/ when you want clean separation or plan to allow side-by-side versions / complete replacement.
Use /etc/ only for configuration.
Use /var/ only for runtime-generated data.
This structure keeps your distribution clean, maintainable, and compatible with packaging tools, sysadmin expectations, and future porting efforts.
