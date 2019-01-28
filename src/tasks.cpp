#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QFile>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <QProcess>
#include <QMenu>
#include <QAction>
#include <QProcess>
#include <QScreen>
#include <QApplication>
#include <QGraphicsDropShadowEffect>

#include <QX11Info>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusConnectionInterface>
#include <QDBusReply>
#include <QPainter>

#include <xcb/xproto.h>
#include <xcb/xcb.h>
#include <xcb/xcb_image.h>
#include <xcb/xcb_util.h>
#include <xcb/xfixes.h>

#include <algorithm>

#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/NETWM>
#include <KF5/KConfigCore/KConfigGroup>

#include "shell.h"
#include "tasks.h"
#include "frame.h"

namespace {
template <typename T> using CScopedPointer = QScopedPointer<T, QScopedPointerPodDeleter>;
struct ScopedPointerXcbImageDeleter
{
    static inline void cleanup(xcb_image_t *xcbImage) {
        if (xcbImage) {
            xcb_image_destroy(xcbImage);
        }
    }
};
}

static const bool isKWinAvailable() {
    if (QDBusConnection::sessionBus().interface()->isServiceRegistered(QStringLiteral("org.kde.KWin"))) {
        QDBusInterface interface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Effects"), QStringLiteral("org.kde.kwin.Effects"));
        QDBusReply<bool> reply = interface.call(QStringLiteral("isEffectLoaded"), "screenshot");

        return reply.value();
    }

    return false;
};

// ----------

Q::Task::Task(Q::Tasks *tasks, const QString &name, const QString &classClass)
    : QPushButton(tasks),
    Model(name, tasks->shell()),
    myParent(tasks),
    myClassClass(classClass),
    myName(name),
    mySize(QSize(48, 48)) {
    setIconSize(mySize);
    setMinimumSize(mySize);
    populateContextMenu();

    if(myParent->previewTasks())
        myTaskPreview = new TaskPreview(this);
};

// Configurations
void Q::Task::save(KConfigGroup *grp)
{
    if(!pinned)
        grp->deleteGroup();
    else if(!grp->exists())
    {
        grp->writeEntry("Type", "Task");
        grp->writeEntry("Command", myCommand);
        grp->writeEntry("Class", myClassClass);
        grp->writeEntry("Icon", myName);
    }
};

void Q::Task::load(KConfigGroup *grp)
{
    myCommand = grp->readEntry("Command", "");
    myClassClass = grp->readEntry("Class", "");

    QString iconName = grp->readEntry("Icon", myName);
    setIcon(QIcon::fromTheme(iconName));

    int size = grp->readEntry("Size", myParent->size());
    mySize = QSize(size, size);
    setIconSize(mySize);
    setMinimumSize(mySize);
};

// Commands
void Q::Task::runCommand() {
    myProcess.startDetached(myCommand, myArguments);
};

void Q::Task::setCommand(QString command) {
    QStringList args = command.split(" ");
    myCommand = args.first();
    QStringList arguments(args);
    arguments.removeFirst();
    myArguments = arguments;
};

// Windows
void Q::Task::addWindow(WId wid) {
    if(!myWindows.contains(wid))
        myWindows.append(wid);
    if(myParent->previewTasks())
        myTaskPreview->addWindow(wid);
};

void Q::Task::removeWindow(WId wid) {
    myWindows.removeAll(wid);
    if(myWindows.isEmpty() && !pinned)
        myParent->removeTask(this);
    if(myParent->previewTasks())
        myTaskPreview->removeWindow(wid);
};

void Q::Task::removeAllWindows() {
    myWindows.clear();
};

void Q::Task::closeAllWindows() {
    foreach(WId wid, myWindows)
        NETRootInfo(QX11Info::connection(), NET::CloseWindow).closeWindowRequest(wid);
};

// Mouse
void Q::Task::mousePressEvent(QMouseEvent *event) {

};

void Q::Task::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        if(pinned && myWindows.isEmpty())
            runCommand();
        else if(myWindows.count() > 1) {
            populateWindowsContextMenu();
            myWindowsContextMenu.popup(getContextMenuPos(&myWindowsContextMenu));
        } else if(KWindowSystem::activeWindow() == myWindows.first())
            KWindowSystem::minimizeWindow(myWindows.first());
        else
            KWindowSystem::forceActiveWindow(myWindows.first());
    } else if(event->button() == Qt::RightButton) {
        populateContextMenu();
        myContextMenu.popup(getContextMenuPos(&myContextMenu));
    }
};

void Q::Task::enterEvent(QEvent *) {
    if(myParent->previewTasks())
        myParent->hideAllPreviews();
    if(!myWindows.isEmpty()) {
        if(myParent->previewTasks()) {
            connect(myParent->timer(), &QTimer::timeout, this, [this](){ // performance reasons + rid of the annoyance if you just want to use the dash
                if(underMouse()) {
                    myTaskPreview->move(getContextMenuPos(myTaskPreview));
                    myTaskPreview->show();
                }
                myParent->timer()->disconnect(this);
            });
        } else {
            populateWindowsContextMenu();
            myWindowsContextMenu.popup(getContextMenuPos(&myWindowsContextMenu));
        }
    }
};

void Q::Task::leaveEvent(QEvent *) {
    myWindowsContextMenu.hide();
};

QPoint Q::Task::getContextMenuPos(QWidget *widget) {
    widget->setAttribute(Qt::WA_DontShowOnScreen, true);
    widget->show();
    widget->hide();
    widget->setAttribute(Qt::WA_DontShowOnScreen, false);

    QPoint p = myParent->parentWidget()->parentWidget()->pos();
    PanelPosition position = static_cast<Panel*>(myParent->parentWidget()->parentWidget())->position();
    if(position == PanelPosition::Bottom) {
        p.setX(p.x() + x());
        p.setY(p.y() - y() - widget->height());
    } else if(position == PanelPosition::Left || position == PanelPosition::Right) {
        p.setX(p.x() + myParent->parentWidget()->width());
        p.setY(p.y() + y());
    } else {
        p.setX(p.x() + x());
        p.setY(p.y());
    }
    return p;
};

// Context Menu
void Q::Task::populateContextMenu() {
    myContextMenu.clear();
    QAction *act;

    act = new QAction("Open new instance");
    connect(act, SIGNAL(triggered()), this, SLOT(runCommand()));
    myContextMenu.addAction(act);

    if(pinned) {
        act = new QAction(QIcon::fromTheme("unpin"), "Unpin from panel");
        connect(act, SIGNAL(triggered()), this, SLOT(unpin()));
        myContextMenu.addAction(act);
    } else {
        act = new QAction(QIcon::fromTheme("pin"), "Pin to panel");
        connect(act, SIGNAL(triggered()), this, SLOT(pin()));
        myContextMenu.addAction(act);
    }

    if(!myWindows.isEmpty()) {
        act = new QAction(QIcon::fromTheme("exit"), "Close all windows");
        connect(act, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
        myContextMenu.addAction(act);
    }
};

void Q::Task::populateWindowsContextMenu() {
    myWindowsContextMenu.clear();
    QAction *act;
    foreach (const WId wid, myWindows) {
        NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMName|NET::WMIcon|NET::WMState, 0);
        act = new QAction(info.name(), &myWindowsContextMenu);
        NETIcon icon = info.icon();
        if(icon.size.width && icon.size.height) {
            QImage image(icon.data, icon.size.width, icon.size.height, QImage::Format_ARGB32);
            act->setIcon(QPixmap::fromImage(image));
        }
        connect(act, &QAction::triggered, [wid]() {
            KWindowSystem::forceActiveWindow(wid);
        });
        myWindowsContextMenu.addAction(act);
    }
};

// pinned
void Q::Task::pin() {
    pinned = true;
    myParent->shell()->save(this);
    myParent->shell()->save(myParent);
};

void Q::Task::unpin() {
    if(myWindows.isEmpty()) myParent->removeTask(this);
    else pinned = false;
    myParent->shell()->save(this);
    myParent->shell()->save(myParent);
};

// ----------

Q::TaskPreview::TaskPreview(Q::Task *task) : Q::Frame(), myTask(task) {
    setWindowFlags(Qt::ToolTip);
    setLayout(new QBoxLayout(static_cast<QBoxLayout*>(myTask->parentWidget()->layout())->direction()));
    layout()->setSizeConstraint(QLayout::SetFixedSize);
    resize(0, 0);
};

// events
void Q::TaskPreview::showEvent(QShowEvent*) {
    Display *display = QX11Info::display();
    Atom atom;

    QVarLengthArray<long, 1024> data(4);

    PanelPosition position = static_cast<Panel*>(myTask->tasks()->parentWidget()->parentWidget())->position();
    Shell *shell = static_cast<Panel*>(myTask->tasks()->parentWidget()->parentWidget())->shell();
    if(position == PanelPosition::Left)
        data[0] = shell->getStrutLeft();
    else if(position == PanelPosition::Right)
        data[0] = shell->getStrutRight();
    else if(position == PanelPosition::Top)
        data[0] = shell->getStrutTop();
    else
        data[0] = shell->getStrutBottom();
    data[1] = (int)position;
    data[2] = 200;
    data[3] = 200;

    atom = XInternAtom(display, "_KDE_SLIDE", false);
    XChangeProperty(display, winId(), atom, atom, 32, PropModeReplace,
            reinterpret_cast<unsigned char *>(data.data()), data.size());

    KWindowSystem::setState(winId(), NET::SkipTaskbar);
};

void Q::TaskPreview::leaveEvent(QEvent *) {
    hide();
};

// slots
void Q::TaskPreview::addWindow(WId wid) {
    if(wids.contains(wid))
        return;

    wids.append(wid);
    WindowPreview *preview = new WindowPreview(wid);
    myPreviews.append(preview);
    layout()->addWidget(preview);
};

void Q::TaskPreview::removeWindow(WId wid) {
    foreach(WindowPreview *preview, myPreviews) {
        if(preview->wid() == wid) {
            wids.removeAll(preview->wid());
            myPreviews.removeAll(preview);
            layout()->removeWidget(preview);
            delete preview;
            return;
        }
    }
};

// ----------
Q::WindowPreview::WindowPreview(WId wid) : QWidget(), myWid(wid) {
    layout = new QVBoxLayout(this);
    setLayout(layout);

    title = new QLabel();
    title->setStyleSheet("color: white; max-width: 240px; margin: 0 5px; margin-top: 10px;");
    title->setWordWrap(true);
    layout->addWidget(title);

    QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);
    effect->setColor(QColor("#000000"));
    effect->setBlurRadius(10);
    effect->setOffset(0, 0);
    title->setGraphicsEffect(effect);

    QScreen *screen = QGuiApplication::primaryScreen();
    window = new QLabel();
    window->setStyleSheet("margin: 0 10px; margin-bottom: 10px;");
    //window->resize(250,250);
    layout->addWidget(window);

    layout->addStretch();

    grabWindow();

    connect(this, &Q::WindowPreview::pixmapChanged, [this](QPixmap pixmap){
        window->setPixmap(pixmap.scaledToWidth(220));
    });
};

// events
void Q::WindowPreview::showEvent(QShowEvent*) {
    NETWinInfo info(QX11Info::connection(), myWid, QX11Info::appRootWindow(), NET::WMName, 0);
    const char *name = info.name();
    if(name == nullptr) {
        title->hide();
        layout->setSpacing(0);
        layout->setMargin(0);
        setStyleSheet("padding: 10px;");
        window->setStyleSheet("margin: 5px;");
    } else {
        title->show();
        title->setText(name);
    }
    grabWindow();
};

void Q::WindowPreview::mouseReleaseEvent(QMouseEvent *event) {
    KWindowSystem::forceActiveWindow(myWid);
};

// Code from KDE's spectacle
// TODO might need to move this to another file
static QPixmap convertFromNative(xcb_image_t *xcbImage) {
    QImage::Format format = QImage::Format_Invalid;

    switch (xcbImage->depth) {
    case 1:
        format = QImage::Format_MonoLSB;
        break;
    case 16:
        format = QImage::Format_RGB16;
        break;
    case 24:
        format = QImage::Format_RGB32;
        break;
    case 30:
        format = QImage::Format_BGR30;
        break;
    case 32:
        format = QImage::Format_ARGB32_Premultiplied;
        break;
    default:
        return QPixmap(); // we don't know
    }

    // The RGB32 format requires data format 0xffRRGGBB, ensure that this fourth byte really is 0xff
    if (format == QImage::Format_RGB32) {
        quint32 *data = reinterpret_cast<quint32 *>(xcbImage->data);
        for (int i = 0; i < xcbImage->width * xcbImage->height; i++) {
            data[i] |= 0xff000000;
        }
    }

    QImage image(xcbImage->data, xcbImage->width, xcbImage->height, format);

    if (image.isNull()) {
        return QPixmap();
    }

    // work around an abort in QImage::color

    if (image.format() == QImage::Format_MonoLSB) {
        image.setColorCount(2);
        image.setColor(0, QColor(Qt::white).rgb());
        image.setColor(1, QColor(Qt::black).rgb());
    }

    // Image is ready. Since the backing data from xcbImage could be freed
    // before the QPixmap goes away, a deep copy is necessary.
    return QPixmap::fromImage(image).copy();
};

static QPixmap getPixmapFromDrawable(xcb_drawable_t drawableId, const QRect &rect) {
    xcb_connection_t *xcbConn = QX11Info::connection();

    // proceed to get an image based on the geometry (in device pixels)

    QScopedPointer<xcb_image_t, ScopedPointerXcbImageDeleter> xcbImage(
        xcb_image_get(
            xcbConn,
            drawableId,
            rect.x(),
            rect.y(),
            rect.width(),
            rect.height(),
            ~0,
            XCB_IMAGE_FORMAT_Z_PIXMAP
        )
    );

    // too bad, the capture failed.
    if (xcbImage.isNull()) {
        return QPixmap();
    }

    // now process the image

    QPixmap nativePixmap = convertFromNative(xcbImage.data());
    return nativePixmap;
};

static QRect getDrawableGeometry(xcb_drawable_t drawable) {
    xcb_connection_t *xcbConn = QX11Info::connection();

    xcb_get_geometry_cookie_t geomCookie = xcb_get_geometry_unchecked(xcbConn, drawable);
    CScopedPointer<xcb_get_geometry_reply_t> geomReply(xcb_get_geometry_reply(xcbConn, geomCookie, NULL));

    return QRect(geomReply->x, geomReply->y, geomReply->width, geomReply->height);
};

void Q::WindowPreview::grabWindow() {
    if(isKWinAvailable()) {
        QDBusConnection bus = QDBusConnection::sessionBus();
        bus.connect(QStringLiteral("org.kde.KWin"),
                    QStringLiteral("/Screenshot"),
                    QStringLiteral("org.kde.kwin.Screenshot"),
                    QStringLiteral("screenshotCreated"),
                    this, SLOT(KWinDBusScreenshotHelper(quint64)));
        QDBusInterface interface(QStringLiteral("org.kde.KWin"), QStringLiteral("/Screenshot"), QStringLiteral("org.kde.kwin.Screenshot"));

        interface.call(QStringLiteral("screenshotForWindow"), (quint64)myWid, 0);
    }
};

void Q::WindowPreview::KWinDBusScreenshotHelper(quint64 pixmapId) {
    QRect rect = getDrawableGeometry((xcb_drawable_t)pixmapId);
    mPixmap = getPixmapFromDrawable((xcb_drawable_t)pixmapId, rect);
    if (!mPixmap.isNull()) {
        emit pixmapChanged(mPixmap);
    }
};

// ----------

Q::Tasks::Tasks(const QString& name, Q::Shell *parent) : QWidget(), Q::Model(name, parent) {
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);

    myTimer = new QTimer(this);
    myTimer->setInterval(300);
};

// configurations
void Q::Tasks::save(KConfigGroup *grp) {
    QStringList pinned;
    foreach(Task *t, myTasks)
        if(t->isPinned())
            pinned.append(t->name());
    grp->writeEntry("Pinned", pinned);
};

void Q::Tasks::load(KConfigGroup *grp) {
    myPreviewTasks = grp->readEntry("PreviewTasks", true);
    mySize = grp->readEntry("Size", 48);
    static_cast<QBoxLayout*>(layout())->setDirection((QBoxLayout::Direction)grp->readEntry("Direction", 0));

    QStringList pinned = grp->readEntry("Pinned", QStringList());
    foreach(QString pin, pinned) {
        Model *m = shell()->getModelByName(pin, this);
        if(m) {
            Task *t = dynamic_cast<Task*>(m);
            t->setPinned(true);
            addTask(t);
        } else {
            Task *t = new Task(this, pin);
            t->setCommand(pin);
            addTask(t);
        }
    }
    populateWindows();

    connect(KWindowSystem::self(), SIGNAL(windowAdded(WId)), this, SLOT(windowAdded(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowRemoved(WId)), this, SLOT(windowRemoved(WId)));
};

// slots
void Q::Tasks::windowAdded(WId wid) {
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMIcon|NET::WMState, NET::WM2WindowClass);
    if(info.state() & NET::SkipTaskbar)
        return;
    const QString cmdline = getCmdline(wid);
    Task *task = getTask(info.windowClassClass());
    //Task *task = getTaskByClass(info.windowClassClass());
    if(task)
        task->addWindow(wid);
    else {
        task = new Task(this, info.windowClassName(), info.windowClassClass());
        task->setCommand(cmdline);

        NETIcon icon = info.icon();
        if(icon.size.width && icon.size.height) {
            QImage image(icon.data, icon.size.width, icon.size.height, QImage::Format_ARGB32);
            task->setIcon(QPixmap::fromImage(image));
        } else {
            task->setIcon(QIcon::fromTheme(info.windowClassName()));
        }

        task->addWindow(wid);
        addTask(task);
    }
};

void Q::Tasks::windowRemoved(WId wid) {
    foreach (Task *task, myTasks)
        task->removeWindow(wid);
    myWindows.removeAll(wid);
};

void Q::Tasks::populateWindows() {
    myWindows = QList<WId>(KWindowSystem::windows());
    foreach (WId wid, myWindows)
        windowAdded(wid);
};

void Q::Tasks::enterEvent(QEvent *) {
    myTimer->start();
};

void Q::Tasks::leaveEvent(QEvent *) {
    myTimer->disconnect();
    myTimer->stop();
};

// tasks
Q::Task *Q::Tasks::getTask(const QString &classClass) const {
    foreach (Task *task, myTasks)
        if(task->classClass() == classClass)
            return task;
    return 0;
};

void Q::Tasks::addTask(Task *t) {
    t->setIconSize(QSize(mySize,mySize));
    t->setMinimumSize(QSize(mySize,mySize));
    boxLayout()->addWidget(t);
    myTasks << t;
};

void Q::Tasks::removeTask(Task *t) {
    if(myTasks.contains(t)) {
        boxLayout()->removeWidget(t);
        myTasks.removeAll(t);
        t->deleteLater();
    }
};

// utilities
static QString whichCmd(const QString &cmd) { // emulated the which command for programs started in cmd
    QString bin;
    for(int i = 0; i < cmd.size(); i++) {
        const QChar ch = cmd[i];
        if(ch == '\\')
            i++;
        else if (ch == ' ')
            break;
        else
            bin += ch;
    }

    if(bin.startsWith("/"))
        return bin;
    QStringList pathEnv = QString(qgetenv("PATH").constData()).split(":");
    foreach (QString path, pathEnv) {
        if(QFile::exists(path + "/" + bin)) {
            return path + "/" + bin;
        }
    }
    return bin;
};

QString Q::Tasks::getCmdline(WId wid) {
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMPid, 0);
    QFile file(QString("/proc/") + QString::number(info.pid()) + QString("/cmdline"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return whichCmd(QString::fromUtf8(file.readAll()));
};

void Q::Tasks::hideAllPreviews() {
    foreach(Task *task, myTasks) {
        TaskPreview *preview = task->taskPreview();
        if(preview) {
            preview->hide();
        }
    }
};
