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
sudo apt install extra-cmake-modules qtbase5-dev libx11-dev libkf5crash-dev libkf5kio-dev libkf5solid-dev libkf5jobwidgets-dev libkf5textwidgets-dev libkf5bookmarks-dev libkf5xmlgui-dev libkf5itemviews-dev libkf5attica-dev libkf5sonnet-dev libkf5globalaccel-dev libkf5guiaddons-dev libkf5codecs-dev libkf5auth-dev libkf5dbusaddons-dev libkf5coreaddons-dev libkf5iconthemes-dev libkf5configwidgets-dev libkf5widgetsaddons-dev libkf5service-dev libkf5config-dev libkf5windowsystem-dev
```

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
sudo make install
```

## Run

To replace plasmashell with qshell, run the command:

```
kquitapp plasmashell; sleep 2; qshell
```

## Install

Although there is an xsession file, it currently doesn't work. To run the shell on startup, you should hijack Plasma's `.desktop` files. To do this, replace `/etc/xdg/autostart/org.kde.plasmashell.desktop` with `org.kde.plasmashell.desktop` in this repository. You can then log in to Q::Shell in your display manager by choosing the KDE Plasma session.

**NOTE**: there is currently a login delay by doing this, I'm currently investigating this, however bear that in mind.

## Credits

 * Qt and KF5
 * [BE::Shell](https://sourceforge.net/projects/be-shell/)
 * [pamixer](https://github.com/cdemoulins/pamixer/)
