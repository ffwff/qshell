#include <QWidget>
#include <QHBoxLayout>
#include <QLabel>
#include <QX11Info>
#include <QIcon>
#include <QPushButton>
#include <QMenu>
#include <QAction>
#include <QKeySequence>
#include <QApplication>
#include <QScreen>
#include <QTimer>
#include <QMouseEvent>
#include <QDebug>

#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/NETWM>
#include <KF5/KWindowSystem/KWindowInfo>

#include "winctrl.h"
#include "model.h"
#include "shell.h"

static void toggleMaximize() {
    NETWinInfo info(QX11Info::connection(), KWindowSystem::activeWindow(), QX11Info::appRootWindow(), NET::WMState, 0);
    if(info.state() & NET::Max)
        KWindowSystem::clearState(KWindowSystem::activeWindow(), NET::Max);
    else
        KWindowSystem::setState(KWindowSystem::activeWindow(), NET::Max);
}

static void closeWindow() {
    NETRootInfo(QX11Info::connection(), NET::CloseWindow).closeWindowRequest(KWindowSystem::activeWindow());
}

static void minimize() {
    KWindowSystem::minimizeWindow(KWindowSystem::activeWindow());
}

// ----------

Q::WinTitle::WinTitle(QWidget *parent) : QLabel(parent), timer(0) {
    populateContextMenu();
}

// Clicks
void Q::WinTitle::doubleClick() {
    toggleMaximize();
}

void Q::WinTitle::click() {
    KWindowInfo info(KWindowSystem::activeWindow(), NET::WMState);
    if(info.state() & NET::SkipTaskbar)
        return;
    populateContextMenu();
    QRect geometry = QGuiApplication::primaryScreen()->geometry();
    Shell *shell = static_cast<WinCtrl*>(parentWidget())->shell();
    myContextMenu.popup(QPoint(
        std::min(shell->getStrutLeft() + parentWidget()->x(), geometry.width() - width()),
        parentWidget()->y() + parentWidget()->parentWidget()->height()
    ));
}

// Mouse Events
void Q::WinTitle::mousePressEvent(QMouseEvent *) {
    isDoubleClick = false;
}

void Q::WinTitle::mouseDoubleClickEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton)
        isDoubleClick = true;
}

void Q::WinTitle::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton)
        click();
    else if(!isDoubleClick) {
        timer = new QTimer();
        timer->setInterval(QApplication::doubleClickInterval() * 0.5);
        timer->setSingleShot(true);
        connect(timer, &QTimer::timeout, [this]() {
            click();
        });
        timer->start();
        return;
    } else {
        doubleClick();
    }

    if(timer)
        timer->stop();

}

// Context Menu
void Q::WinTitle::populateContextMenu() {
    KWindowInfo info(KWindowSystem::activeWindow(), NET::WMName|NET::WMState|NET::WMDesktop);

    myContextMenu.clear();

    QAction *act;

    QMenu *menu = myContextMenu.addMenu("Move to desktop");

    act = new QAction("All desktops");
    act->setCheckable(true);
    act->setChecked(info.onAllDesktops());
    connect(act, &QAction::triggered, [](){
        KWindowSystem::setOnAllDesktops(KWindowSystem::activeWindow(), true);
    });
    menu->addAction(act);

    menu->addSeparator();

    for(int i = 1; i <= KWindowSystem::numberOfDesktops(); i++) {
        act = new QAction(KWindowSystem::desktopName(i));
        act->setCheckable(true);
        act->setChecked(info.isOnDesktop(i));
        connect(act, &QAction::triggered, [i](){
            KWindowSystem::setOnDesktop(KWindowSystem::activeWindow(), i);
        });
        menu->addAction(act);
    }

    myContextMenu.addSeparator();

    act = new QAction(QIcon::fromTheme("up"), "Keep above others");
    act->setCheckable(true);
    act->setChecked(info.state() & NET::StaysOnTop);
    connect(act, &QAction::triggered, [info](){
        if(info.state() & NET::StaysOnTop)
            KWindowSystem::clearState(KWindowSystem::activeWindow(), NET::StaysOnTop);
        else
            KWindowSystem::setState(KWindowSystem::activeWindow(), info.state() | NET::StaysOnTop);
    });
    myContextMenu.addAction(act);

    act = new QAction(QIcon::fromTheme("down"), "Keep below others");
    act->setCheckable(true);
    act->setChecked(info.state() & NET::KeepBelow);
    connect(act, &QAction::triggered, [info](){
        if(info.state() & NET::KeepBelow)
            KWindowSystem::clearState(KWindowSystem::activeWindow(), NET::KeepBelow);
        else
            KWindowSystem::setState(KWindowSystem::activeWindow(), info.state() | NET::KeepBelow);
    });
    myContextMenu.addAction(act);

    myContextMenu.addSeparator();

    act = new QAction(QIcon::fromTheme("window-minimize-symbolic"), "Maximize");
    connect(act, &QAction::triggered, [](){ minimize(); });
    myContextMenu.addAction(act);

    if(info.state() & NET::Max) {
        act = new QAction(QIcon::fromTheme("window-restore-symbolic"), "Unmaximize");
        connect(act, &QAction::triggered, [](){ toggleMaximize(); });
        myContextMenu.addAction(act);
    } else {
        act = new QAction(QIcon::fromTheme("window-maximize-symbolic"), "Maximize");
        connect(act, &QAction::triggered, [](){ toggleMaximize(); });
        myContextMenu.addAction(act);
    }

    act = new QAction(QIcon::fromTheme("window-close-symbolic"), "Close");
    act->setShortcut(QKeySequence("Alt+F4"));
    connect(act, &QAction::triggered, [](){ closeWindow(); });
    myContextMenu.addAction(act);
}

// ----------

Q::WinCtrl::WinCtrl(const QString& name, Shell* shell)
    : QWidget(), Model(name, shell) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setContentsMargins(0,0,0,0);
    setLayout(layout);

    closeBtn = new QPushButton();
    closeBtn->setIcon(QIcon::fromTheme("window-close-symbolic"));
    minimizeBtn = new QPushButton();
    minimizeBtn->setIcon(QIcon::fromTheme("window-minimize-symbolic"));
    maximizeBtn = new QPushButton();
    maximizeBtn->setIcon(QIcon::fromTheme("window-restore-symbolic"));

    label = new WinTitle(this);
    update();

    connect(KWindowSystem::self(), SIGNAL(activeWindowChanged(WId)), this, SLOT(update(WId)));
    connect(KWindowSystem::self(), SIGNAL(windowChanged(WId, const unsigned long*)), this, SLOT(update()));
    connect(closeBtn, &QPushButton::clicked, [this]() {
        closeWindow();
    });
    connect(minimizeBtn, &QPushButton::clicked, [this]() {
        minimize();
        update();
    });
    connect(maximizeBtn, &QPushButton::clicked, []() {
        toggleMaximize();
    });
}

void Q::WinCtrl::load(KConfigGroup *grp) {
    const QString &controls = grp->readEntry("Controls", "XMmt");
    for(int i = 0; i < controls.size(); i++) {
        const QChar &c = controls[i];
        if(c == 'X') boxLayout()->addWidget(closeBtn);
        else if(c == 'M') boxLayout()->addWidget(maximizeBtn);
        else if(c == 'm') boxLayout()->addWidget(minimizeBtn);
        else if(c == 't') boxLayout()->addWidget(label);
    }
}

void Q::WinCtrl::update(WId wid) {
    KWindowInfo info(wid, NET::WMName|NET::WMState);
    label->setText(info.name());
    if(info.state() & NET::Max) {
        closeBtn->show();
        minimizeBtn->show();
        maximizeBtn->show();
    } else {
        closeBtn->hide();
        minimizeBtn->hide();
        maximizeBtn->hide();
    }
}
