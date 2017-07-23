#include <QWidget>
#include <QPushButton>
#include <QList>
#include <QFile>
#include <QX11Info>
#include <QDebug>
#include <QIcon>
#include <QPixmap>
#include <QImage>
#include <QMouseEvent>
#include <QTimer>
#include <QProcess>
#include <QMenu>
#include <QAction>

#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/NETWM>
#include <KF5/KConfigCore/KConfigGroup>

#include "shell.h"
#include "tasks.h"

Q::Task::Task(Q::Tasks *tasks, const QString &name) :
QPushButton(static_cast<QWidget *>(tasks)),
Model(name, tasks->shell()),
myParent(tasks),
myName(name),
myCommand(""),
mySize(QSize(48, 48)),
pinned(false)
{
    setIconSize(mySize);
    setMinimumSize(mySize);
    populateContextMenu();
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
    }
};

void Q::Task::load(KConfigGroup *grp)
{
    myCommand = grp->readEntry("Command", "");
    
    QString iconName = grp->readEntry("Icon", myName);
    setIcon(QIcon::fromTheme(iconName));
    
    int size = grp->readEntry("Size", myParent->size());
    mySize = QSize(size, size);
    setIconSize(mySize);
    setMinimumSize(mySize);
};

// Commands
void Q::Task::runCommand()
{
    myProcess.startDetached(myCommand, myArguments);
};

void Q::Task::setCommand(QString command)
{
    QStringList args = command.split(" ");
    myCommand = args.first();
    QStringList arguments(args);
    arguments.removeFirst();
    myArguments = arguments;
};

// Windows
void Q::Task::addWindow(WId wid)
{
    if(!myWindows.contains(wid))
        myWindows.append(wid);
};

void Q::Task::removeWindow(WId wid)
{
    myWindows.removeAll(wid);
    if(myWindows.isEmpty() && !pinned)
        myParent->removeTask(this);
};

void Q::Task::removeAllWindows()
{
    myWindows.clear();
};

void Q::Task::closeAllWindows()
{
    foreach(WId wid, myWindows)
        NETRootInfo(QX11Info::connection(), NET::CloseWindow).closeWindowRequest(wid);
};

// Mouse
void Q::Task::mousePressEvent(QMouseEvent *event)
{
    
};

void Q::Task::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        if(pinned && myWindows.isEmpty())
            runCommand();
        else if(myWindows.count() > 1)
        {
            populateWindowsContextMenu();
            QPoint p = myParent->parentWidget()->pos();
            p.setX(p.x() + pos().x());
            p.setY(p.y() - myWindowsContextMenu.sizeHint().height());
            myWindowsContextMenu.popup(p);
        }
        else if(KWindowSystem::activeWindow() == myWindows.first())
            KWindowSystem::minimizeWindow(myWindows.first());
        else
            KWindowSystem::forceActiveWindow(myWindows.first());
    }
    else if(event->button() == Qt::RightButton)
    {
        populateContextMenu();
        QPoint p = myParent->parentWidget()->pos();
        p.setX(p.x() + pos().x());
        p.setY(p.y() - myContextMenu.sizeHint().height());
        myContextMenu.popup(p);
    }
};

// Context Menu
void Q::Task::populateContextMenu()
{
    myContextMenu.clear();
    QAction *act;
    
    act = new QAction("Open new instance");
    connect(act, SIGNAL(triggered()), this, SLOT(runCommand()));
    myContextMenu.addAction(act);
    
    if(pinned)
    {
        act = new QAction(QIcon::fromTheme("unpin"), "Unpin from panel");
        connect(act, SIGNAL(triggered()), this, SLOT(unpin()));
        myContextMenu.addAction(act);
    }
    else
    {
        act = new QAction(QIcon::fromTheme("pin"), "Pin to panel");
        connect(act, SIGNAL(triggered()), this, SLOT(pin()));
        myContextMenu.addAction(act);
    }
    
    if(!myWindows.isEmpty())
    {
        act = new QAction(QIcon::fromTheme("exit"), "Close all windows");
        connect(act, SIGNAL(triggered()), this, SLOT(closeAllWindows()));
        myContextMenu.addAction(act);
    }
};

void Q::Task::populateWindowsContextMenu()
{
    myWindowsContextMenu.clear();
    QAction *act;
    foreach (WId wid, myWindows)
    {
        NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMName|NET::WMIcon|NET::WMState, 0);
        act = new QAction(info.name());
        NETIcon icon = info.icon();
        if(icon.size.width && icon.size.height)
        {
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
void Q::Task::pin()
{
    pinned = true;
    myParent->shell()->save(this);
    myParent->shell()->save(myParent);
};

void Q::Task::unpin()
{
    if(myWindows.isEmpty())
        myParent->removeTask(this);
    else
        pinned = false;
    myParent->shell()->save(this);
    myParent->shell()->save(myParent);
};

// ----------

Q::Tasks::Tasks(const QString& name, Q::Shell *parent) : QWidget(), Q::Model(name, parent)
{
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
};

// configurations
void Q::Tasks::save(KConfigGroup *grp)
{
    QStringList pinned;
    foreach(Task *t, myTasks)
        if(t->isPinned())
            pinned.append(t->name());
    grp->writeEntry("Pinned", pinned);
};

void Q::Tasks::load(KConfigGroup *grp)
{
    mySize = grp->readEntry("Size", 48);
    static_cast<QBoxLayout*>(layout())->setDirection((QBoxLayout::Direction)grp->readEntry("Direction", 0));
    
    QStringList pinned = grp->readEntry("Pinned", QStringList());
    foreach(QString pin, pinned)
    {
        Model *m = shell()->getModelByName(pin, this);
        if(m)
        {
            Task *t = dynamic_cast<Task*>(m);
            t->setPinned(true);
            addTask(t);
        }
        else
        {
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
void Q::Tasks::windowAdded(WId wid)
{
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMIcon|NET::WMState, NET::WM2WindowClass);
    if(info.state() & NET::SkipTaskbar)
        return;
    QString cmdline = getCmdline(wid);
    Task *task = getTaskByCommand(cmdline);
    if(task)
        task->addWindow(wid);
    else
    {
        task = new Task(this, cmdline.split("/").last());
        task->setCommand(cmdline);
        
        NETIcon icon = info.icon();
        if(icon.size.width && icon.size.height)
        {
            QImage image(icon.data, icon.size.width, icon.size.height, QImage::Format_ARGB32);
            task->setIcon(QPixmap::fromImage(image));
        } else
            task->setIcon(QIcon::fromTheme(info.windowClassName()));
        
        task->addWindow(wid);
        addTask(task);
    }
};

void Q::Tasks::windowRemoved(WId wid)
{
    foreach (Task *task, myTasks)
        task->removeWindow(wid);
    myWindows.removeAll(wid);
};

void Q::Tasks::populateWindows()
{
    myWindows = QList<WId>(KWindowSystem::windows());
    foreach (WId wid, myWindows)
        windowAdded(wid);
};

// tasks
Q::Task *Q::Tasks::getTaskByCommand(const QString &command)
{
    foreach (Task *task, myTasks)
        if(task->command() == command)
            return task;
    return 0;
};

void Q::Tasks::addTask(Task *t)
{
    t->setIconSize(QSize(mySize,mySize));
    t->setMinimumSize(QSize(mySize,mySize));
    boxLayout()->addWidget(t);
    myTasks << t;
};

void Q::Tasks::removeTask(Task *t)
{
    if(myTasks.contains(t))
    {
        boxLayout()->removeWidget(t);
        myTasks.removeAll(t);
        t->deleteLater();
    }
};

// utilities
QString whichCmd(QString cmd) // emulated the which command for programs started in cmd
{
    if(cmd.startsWith("/"))
        return cmd;
    QStringList pathEnv = QString(qgetenv("PATH").constData()).split(":");
    foreach (QString path, pathEnv)
    {
        if(QFile::exists(path + "/" + cmd))
        {
            return path + "/" + cmd;
        }
    }
    return cmd;
};

QString Q::Tasks::getCmdline(WId wid)
{
    NETWinInfo info(QX11Info::connection(), wid, QX11Info::appRootWindow(), NET::WMPid, 0);
    QFile file(QString("/proc/") + QString::number(info.pid()) + QString("/cmdline"));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return QString();
    return whichCmd(QString::fromUtf8(file.readAll()));
};
