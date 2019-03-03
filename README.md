# Q::Shell

![Screenshot](/screenshots/3.png)

a simple desktop shell based on KF5 and QT5, inspired by plasmashell and BE::Shell.

## Build dependencies

You will need your distro's equivalent to:

- `extra-cmake-modules`
- Qt5 (Core, DBus, Widgets, X11Extras components)
- KF5 (Config, KIO, WindowSystem components)
- KF5Solid
- Xlib
- Pulseaudio

It is recommended to install KDE Plasma along side this as Q::Shell utilizes many of Plasma's features.

For Ubuntu/Debian-based systems, this command should be sufficient:

```
sudo apt install extra-cmake-modules qtbase5-dev libx11-dev libkf5crash-dev libkf5kio-dev libkf5solid-dev libkf5jobwidgets-dev libkf5textwidgets-dev libkf5bookmarks-dev libkf5xmlgui-dev libkf5itemviews-dev libkf5attica-dev libkf5sonnet-dev libkf5globalaccel-dev libkf5guiaddons-dev libkf5codecs-dev libkf5auth-dev libkf5dbusaddons-dev libkf5coreaddons-dev libkf5iconthemes-dev libkf5configwidgets-dev libkf5widgetsaddons-dev libkf5service-dev libkf5config-dev libkf5windowsystem-dev libqt5concurrent5 libpulse-dev
```

You'll also need `dex` installed if you want to open applications in the dash.

For Arch, use this:

```
sudo pacman -S extra-cmake-modules plasma-meta
```

## Build

```
mkdir build
cd build
cmake ..
make
```

## Run

You will need a config file for qshell to work, simply copy-paste one (and its corresponding stylesheet) to your `~/.config` directory. To replace plasmashell with qshell, run the command:

```
kquitapp plasmashell; sleep 2; qshell
```

## Install

```
sudo make install
```

A Q::Shell session will appear on your display manager's session selection.

**NOTE**: it is currently required that KDE Plasma be installed as the session requires several Plasma-specific utilities.

## Credits

 * Qt and KF5
 * [BE::Shell](https://sourceforge.net/projects/be-shell/)
