# Q::Shell

![Screenshot](/screenshots/2.png)

a simple desktop shell based on KF5 and QT5, inspired by plasmashell and BE::Shell.

## Build dependencies

```
sudo apt install qtbase5-dev libx11-dev libx11-xcb-dev libkf5crash-dev libkf5kio-dev libkf5solid-dev libkf5jobwidgets-dev libkf5textwidgets-dev libkf5bookmarks-dev libkf5xmlgui-dev libkf5itemviews-dev libkf5attica-dev libkf5sonnet-dev libkf5globalaccel-dev libkf5guiaddons-dev libkf5codecs-dev libkf5auth-dev libkf5dbusaddons-dev libkf5coreaddons-dev libkf5iconthemes-dev libkf5configwidgets-dev libkf5widgetsaddons-dev libkf5service-dev libkf5config-dev libkf5windowsystem-dev
```

## Build

```
mkdir build
cd build
cmake ..
make
sudo make install
```

Standard stuff.

## Credits

* Qt and KF5 for glorious framework
* BE::Shell for inspiration and some code
* [pamixer](https://github.com/cdemoulins/pamixer/)
* [screenshot code from spectacle](https://www.kde.org/applications/graphics/ksnapshot)
