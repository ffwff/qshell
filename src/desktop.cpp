#include <QLabel>
#include <QWidget>
#include <QApplication>
#include <QScreen>
#include <QImageReader>
#include <QMessageBox>
#include <QDir>
#include <QPainter>
#include <QDebug>
#include <QImage>
#include <QLinearGradient>
#include <QMenu>
#include <QAction>
#include <QMouseEvent>
#include <QFileDialog>
#include <QStandardPaths>
#include <QPushButton>
#include <QWheelEvent>
#include <QSharedPointer>

#include <KF5/KWindowSystem/KWindowSystem>
#include <KF5/KWindowSystem/KWindowEffects>

#include "model.h"
#include "desktop.h"
#include "panel.h"

Q::DesktopIcon::DesktopIcon(const QString& name, Shell *shell)
    : QPushButton(), Q::Model(name, shell) {
}

// Commands
void Q::DesktopIcon::runCommand() {
    myProcess.startDetached(myCommand, myArguments);
}

// Configurations
void Q::DesktopIcon::save(KConfigGroup *grp) {
    if(!pinned)
        grp->deleteGroup();
    else if(!grp->exists()) {
        grp->writeEntry("Type", "DesktopIcon");
        grp->writeEntry("Command", myCommand);
    }
}

void Q::DesktopIcon::load(KConfigGroup *grp) {
    myCommand = grp->readEntry("Command", "");

    QString iconName = grp->readEntry("Icon", myName);
    setIcon(QIcon::fromTheme(iconName));

    int size = grp->readEntry("Size", 0);
    mySize = QSize(size, size);
    setIconSize(mySize);
    resize(mySize);

    myLeft = grp->readEntry("Left", 0);
    myTop = grp->readEntry("Top", 0);
}

// Events

void Q::DesktopIcon::mousePressEvent(QMouseEvent *) {

}

void Q::DesktopIcon::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::LeftButton) {
        runCommand();
    } else if(event->button() == Qt::RightButton) {
//         populateContextMenu();
//         myContextMenu.popup(getContextMenuPos());
    }
}

// ----------

Q::DesktopWallpaperDialog::DesktopWallpaperDialog(Desktop *parent) :
    QFileDialog(parent, "Set desktop background", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)), myParent(parent) {
    connect(this, SIGNAL(fileSelected(const QString &)), this, SLOT(fileSelected(const QString &)));
}

void Q::DesktopWallpaperDialog::fileSelected(const QString &file) {
    myParent->setBackground(file);
    myParent->shell()->save(myParent);
}

// ----------

Q::DesktopShadow::DesktopShadow(Desktop *desktop)
    : QWidget(), myDesktop(desktop) {
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowTransparentForInput | Qt::WindowStaysOnBottomHint);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setFocusPolicy(Qt::NoFocus);
}

void Q::DesktopShadow::paintEvent(QPaintEvent *) {
    // this is pretty HACK
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    KWindowSystem::setOnAllDesktops(winId(), true);
    KWindowEffects::enableBlurBehind(winId(), false);

    QPainter painter(this);
    foreach (auto p, myDesktop->shell()->panels()) {
        // HACK way to make shadows under windows but on top of desktop
        if(p->displaysShadow()) {
            if(p->position() == Q::PanelPosition::Top) {
                QLinearGradient gradient(0, p->y() + p->height(), 0, p->y() + p->height() + 20);
                gradient.setColorAt(0, QColor(0,0,0,64));
                gradient.setColorAt(1, Qt::transparent);
                painter.fillRect(p->x(), p->y() + p->height(), p->width(), p->height() + 20, gradient);
            } else if(p->position() == Q::PanelPosition::Bottom) {
                QLinearGradient gradient(0, p->y() , 0, p->y() - p->height());
                gradient.setColorAt(0, QColor(0,0,0,64));
                gradient.setColorAt(1, Qt::transparent);
                painter.fillRect(p->x(), p->y() - p->height(), p->width(), p->y() + p->height(), gradient);
            } else if(p->position() == Q::PanelPosition::Left) {
                QLinearGradient gradient(p->x() + p->width() + 10, 0, p->x() + p->width(), 0);
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, QColor(0,0,0,64));
                painter.fillRect(p->x() + p->width(),p->y(), p->width(), p->height(), gradient);
            } else {
                QLinearGradient gradient(p->x() - p->width(), 0, p->x() - p->y() + p->width(), 0);
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, QColor(0,0,0,64));
                painter.fillRect(p->x() - p->width(),p->y(), p->width(), p->y() + p->height(), gradient);
            }
        }
    }
}


// ----------

Q::Desktop::Desktop(Shell *shell)
    : QWidget(), Q::Model("Q::Desktop", shell),
    myShadows(new DesktopShadow(this)),
    myDialog(new DesktopWallpaperDialog(this)) {
    iconContainer = new QWidget(this);

    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
//     setScaledContents(true);

    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    populateContextMenu();

    myShadows->resize(QGuiApplication::primaryScreen()->size());

    connect(QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()));
}

// slots
void Q::Desktop::geometryChanged() {
    const auto &size = QGuiApplication::primaryScreen()->size();
    setFixedSize(size);
    myShadows->setFixedSize(size);
    shell()->repaintPanels();
    repaint();
    iconContainer->move(shell()->getStrutLeft() + 5, shell()->getStrutTop() + 5);
    iconContainer->resize(width() - shell()->getStrutLeft() - shell()->getStrutRight() - 5,
                         height() - shell()->getStrutRight() - shell()->getStrutBottom() - 5);
}

void Q::Desktop::showEvent(QShowEvent*) {
    KWindowSystem::setState(myShadows->winId(), NET::SkipTaskbar);
    myShadows->show();
}

void Q::Desktop::paintEvent(QPaintEvent*) {
    QPainter painter(this);
    const auto &size = QGuiApplication::primaryScreen()->geometry();
    const auto &image = myImage.scaled(size.width(), size.height(),Qt::KeepAspectRatioByExpanding,Qt::SmoothTransformation);
    painter.drawImage(0, 0, image);
    myShadows->repaint();
}

// configurations
void Q::Desktop::load(KConfigGroup *group) {
    setBackground(group->readEntry("Background", ""));
    showIcons = group->readEntry("ShowIcons", false);
    myIconSize = group->readEntry("IconSize", 64);
    iconContainer->setVisible(showIcons);
    QStringList icons = group->readEntry("Icons", QStringList());
    foreach (const QString &i, icons) {
        DesktopIcon *icon = static_cast<DesktopIcon*>(shell()->getModelByName(i));
        if(icon) {
            if(!icon->size().width() && !icon->size().height()) {
                icon->setIconSize(QSize(myIconSize,myIconSize));
                icon->setMinimumSize(QSize(myIconSize,myIconSize));
            }
            icon->setParent(iconContainer);
            icon->move(icon->left(), icon->top());
            icon->show();
            myIcons << icon;
        }
    }
}

void Q::Desktop::save(KConfigGroup *group) {
    group->writeEntry("Background", myFileName);
}

// set background
bool Q::Desktop::setBackground(const QString &fileName) {
    if(fileName.isEmpty())
        return false;
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }
    myFileName = fileName;
    myImage = newImage;
    repaint();
    shell()->repaintPanels();
    return true;
}

// events
void Q::Desktop::mouseReleaseEvent(QMouseEvent *event) {
    if(event->button() == Qt::RightButton) {
        myContextMenu.popup(event->pos());
    }
}

void Q::Desktop::wheelEvent(QWheelEvent *we) {
    if (KWindowSystem::numberOfDesktops() < 2)
        return;
    int next = KWindowSystem::currentDesktop();
    const int available = KWindowSystem::numberOfDesktops();
    if (we->delta() < 0)
        ++next;
    else
        --next;
    if (next < 1 )
        next += available;
    if (next > available)
        next -= available;
    KWindowSystem::setCurrentDesktop(next);
}

// ctx menu
void Q::Desktop::populateContextMenu() {
    myContextMenu.clear();

    QAction *act;

    act = new QAction(QIcon::fromTheme("refresh"), "Reload configurations", this);
    connect(act, &QAction::triggered, [this](){ shell()->reloadAll(); });
    myContextMenu.addAction(act);

    myContextMenu.addSeparator();

    act = new QAction(QIcon::fromTheme("preferences-desktop-display"), "Display", this);
    connect(act, &QAction::triggered, [this](){ shell()->kcmshell5("kcm_kscreen"); });
    myContextMenu.addAction(act);

    act = new QAction(QIcon::fromTheme("preferences-desktop-wallpaper"), "Personalize", this);
    connect(act, SIGNAL(triggered()), myDialog, SLOT(show()));
    myContextMenu.addAction(act);
}
