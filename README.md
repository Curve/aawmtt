<hr>

<div align="center"> 
    <h3>
        aawmtt
    </h3>
</div>

<p align="center"> 
    Another AwesomeWM Testing Tool, written in C++20
</p>

---

### New Features

- Live Reload
- "Properly" detect free displays

## Why?

The original [awmtt](https://github.com/gmdfalk/awmtt) as well as [awmtt-ng](https://github.com/basaran/awmtt-ng) have several issues.

1. Both of them seem to have issues with detecting free displays and will straight up ignore the display passed through "-D".

2. For me they both exit instantly when the config is a sym-link or somewhere else than in "~/.config/awesome".

3. Better Live-Reload 

### Packages

- [AUR](https://aur.archlinux.org/packages/aawmtt)

### Dependencies

- A C++20 Compiler *(Tested on Clang 15)*
- Xephyr
- Xlib

### Installation

```sh
mkdir build && cd build
cmake .. && cmake --build . --config Release
```