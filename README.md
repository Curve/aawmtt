<hr>

<div align="center"> 
    <img src="./assets/logo.png" />
    <h3>
        aawmtt
    </h3>
</div>

<p align="center"> 
    Another AwesomeWM Testing Tool, written in C++20
</p>

---

## Features

- Live Reload
- Proper Display detection
- Configurable Restart Mode

## Why?

The original [awmtt](https://github.com/gmdfalk/awmtt) as well as [awmtt-ng](https://github.com/basaran/awmtt-ng) have several issues.

1. awmtt-ng seems to have issues with detecting free displays and will straight up ignore the display passed through "-D".

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

### Usage

```
Usage: 
    aawmtt 
    aawmtt [...]

Options:
    -h --help                   Print usage
    
    -x --xephyr     <path>      Explicitly specify xephyr path 
    --xephyr-args   <args>      Arguments used to invoke xephyr (Default: "-ac -br -noreset")
    
    -a --awesome    <path>      Explicitly specify awesome path
    --awesome-args  <args>      Additional awesome arguments (e.g. "--screen off")
    
    -c --config     <path>      Location of the awesome config to use (Default: "~/.config/awesome/rc.lua")
    -w --watch      <path>      Directory to watch for changes (Default: Parent directory of config)
    -m --mode       <enum>      Awesome restart strategy ["r" = restart, "s" = SIGHUP] (Default: "s")
    
    -s --size       <string>    Xephyr window size (Default: "1920x1080")
    -d --display    <number>    X11 Display to use (Default: Free Display)
    
    --no-reload                 Disable automatic reload
    --no-recursive              Disable recursive directory watch
```

### Screenshots

![screenshot](assets/screenshot.png)
