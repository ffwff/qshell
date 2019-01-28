#include <QApplication>
#include <QDebug>
#include <QLabel>
#include <QRect>
#include <QScreen>
#include <QMap>
#include <QPixmap>
#include <QWindow>
#include <QStandardPaths>
#include <QTimer>

#include <KF5/KConfigCore/KConfigGroup>
#include <KF5/KConfigCore/KSharedConfig>
#include <KF5/KWindowSystem/KWindowSystem>

#include <signal.h>

#include "shell.h"
#include "desktop.h"
#include "panel.h"
#include "tasks.h"
#include "model.h"
#include "dash.h"
#include "volume.h"
#include "network.h"
#include "date.h"
#include "logout.h"
#include "winctrl.h"
#include "trash.h"
#include "battery.h"
#include "mediaplayer.h"
#include "button.h"
#include "systray.h"

Q::ShellApplication::ShellApplication(int argc, char **argv) : QApplication(argc, argv) {
    QCoreApplication::setApplicationName("qshell");
    QGuiApplication::setApplicationDisplayName("Q::Shell Desktop");
    QCoreApplication::setApplicationVersion("0.1");
    myShell = new Shell();
};

// ----------

Q::Shell::Shell() : QWidget( 0, Qt::Window | Qt::FramelessWindowHint )
{
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    KWindowSystem::setOnAllDesktops( winId(), true );
    setMask( QRegion(-1,-1,1,1) );
    show();

    myDesktop = new Desktop(this);
    myDesktop->show();

    myDash = new Dash(this);

    myOneSecond = new QTimer(this);
    myOneSecond->setInterval(1000);

    loadAll();

    connect( QGuiApplication::primaryScreen(), SIGNAL(virtualGeometryChanged(QRect)), this, SLOT(calculateStruts()) );
};

// Configurations
void Q::Shell::saveAll()
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig("qshellrc");

    // models
    foreach (Model *m, myModels) {
        KConfigGroup grp = sharedConfig->group(m->name());
        m->save(&grp);
    }

    // desktop
    KConfigGroup desktopGroup = sharedConfig->group("Q::Desktop");
    myDesktop->save(&desktopGroup);

    // shell
    KConfigGroup shGroup = sharedConfig->group("Q::Shell");
    QStringList list;
    foreach (Panel *p, myPanels)
        list.append(p->name());
    shGroup.writeEntry("Panels", list.join(","));
};

void Q::Shell::save(Model *m)
{
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig("qshellrc");
    KConfigGroup grp = sharedConfig->group(m->name());
    m->save(&grp);
    sharedConfig->sync();
};

void Q::Shell::loadAll() {
    if(!QFile::exists(QStandardPaths::locate(QStandardPaths::ConfigLocation, "qshellrc"))) {
        QFile::copy("/usr/share/qshell/qshellrc", QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qshellrc");
        QFile::copy("/usr/share/qshell/qshell.css", QStandardPaths::writableLocation(QStandardPaths::ConfigLocation) + "/qshell.css");
    }
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig("qshellrc");

    KConfigGroup grp;

    // desktop
    grp = sharedConfig->group("Q::Desktop");
    myDesktop->load(&grp);

    // shell
    KConfigGroup shGroup = sharedConfig->group("Q::Shell");
    QStringList panels = shGroup.readEntry("Panels", QStringList());
    foreach (const QString &panel, panels) {
        Panel *m = static_cast<Panel *>(getModelByName(panel));
        if(m)
            addPanel(m);
    }
    calculateStruts();
    myDesktop->geometryChanged();
    QString styleSheetLocation = shGroup.readEntry("Stylesheet", QString());
    if(!styleSheetLocation.isEmpty()) {
        QFile *file = 0;
        if(QFile::exists(styleSheetLocation))
            file = new QFile(styleSheetLocation);
        else if(!QStandardPaths::locate(QStandardPaths::ConfigLocation, styleSheetLocation).isEmpty())
            file = new QFile(QStandardPaths::locate(QStandardPaths::ConfigLocation, styleSheetLocation));
        if (file && file->open(QIODevice::ReadOnly | QIODevice::Text)) {
            styleSheet = QString::fromUtf8(file->readAll());
            setStyleSheet(styleSheet);
        }
    }

    // dash
    grp = sharedConfig->group("Q::Dash");
    myDash->load(&grp);

    myOneSecond->start();
};

void Q::Shell::reloadAll() {
    myOneSecond->stop();
    myOneSecond->disconnect();
    foreach(Model *m, myModels) {
        QWidget *w = dynamic_cast<QWidget*>(m);
        if(w) w->deleteLater();
    }
    foreach(Panel *p, myPanels)
        p->deleteLater();
    myModels.clear();
    myPanels.clear();
    loadAll();
};

#define COND_LOAD_MODEL(s,m_) else if(type == s) { m = new m_(name, this); static_cast<m_ *>(m)->load(&group); }

Q::Model *Q::Shell::getModelByName(const QString &name, Model *parent) {
    KSharedConfig::Ptr sharedConfig = KSharedConfig::openConfig("qshellrc");
    if(myModels.contains(name))
        return myModels.value(name);
    else if(sharedConfig->hasGroup(name))
    {
        Model *m;
        KConfigGroup group = sharedConfig->group(name);
        QString type = group.readEntry("Type", "");
        if(type.isEmpty())
            return 0;
        COND_LOAD_MODEL("Panel", Panel)
        COND_LOAD_MODEL("Tasks", Tasks)
        else if(type == "Task" && parent) { // prevents loading outside of Tasks
            m = new Task(dynamic_cast<Tasks*>(parent), name);
            static_cast<Task *>(m)->load(&group);
        }
        COND_LOAD_MODEL("DashButton", DashButton)
        else if(type == "Volume") {
            try {
                m = new Volume(name, this);
                static_cast<Volume*>(m)->load(&group);
            } catch(const char *s) {
                qDebug() << s;
                return 0;
            }
        }
        COND_LOAD_MODEL("Systray", Systray)
        COND_LOAD_MODEL("Network", Network)
        COND_LOAD_MODEL("Date", Date)
        COND_LOAD_MODEL("Logout", Logout)
        COND_LOAD_MODEL("WindowControl", WinCtrl)
        COND_LOAD_MODEL("DesktopIcon", DesktopIcon)
        COND_LOAD_MODEL("Trash", Trash)
        COND_LOAD_MODEL("Battery", Battery)
        COND_LOAD_MODEL("MediaPlayer", MediaPlayer)
        COND_LOAD_MODEL("Button", Button)
        else
            return 0;

        myModels.insert(name, m);
        return m;
    }
    else
        return 0;
};

#undef COND_LOAD_MODEL

// Panels
void Q::Shell::addPanel(Q::Panel *panel)
{
    qDebug() << "add panel" << panel->name();
    myPanels.append(panel);
    panel->show();
};

void Q::Shell::repaintPanels() {
    foreach (Q::Panel *panel, myPanels) {
        panel->repaint();
    }
};

// Struts
void Q::Shell::calculateStruts() {
    strut_left   = 0;
    strut_right  = 0;
    strut_top    = 0;
    strut_bottom = 0;
    foreach (Q::Panel *panel, myPanels) {
        if(panel->struts()) {
            if(panel->position() == Q::PanelPosition::Left)
                strut_left += panel->width();
            else if(panel->position() == Q::PanelPosition::Right)
                strut_right += panel->width();
            else if(panel->position() == Q::PanelPosition::Top)
                strut_top += panel->height();
            else
                strut_bottom += panel->height();
        }
    }
    KWindowSystem::setStrut(winId(),strut_left,strut_right,strut_top,strut_bottom);
};

// Kcmshell5
void Q::Shell::kcmshell5(const QString &arg)
{
    QStringList argument;
    argument.append(arg);
    myProcess.startDetached("kcmshell5", argument);
};

// ----------

void signalHandler(int signal)
{
    if (signal == SIGTERM || signal == SIGQUIT || signal == SIGINT)
        QApplication::quit();
}

int main (int argc, char *argv[])
{
    signal(SIGTERM, signalHandler);
    signal(SIGQUIT, signalHandler);
    signal(SIGINT, signalHandler);
    // SIGSEG by KCrash

    Q::ShellApplication app(argc, argv);
    return app.exec();
}
