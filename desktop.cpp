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

#include <KF5/KWindowSystem/KWindowSystem>

#include "model.h"
#include "desktop.h"
#include "panel.h"

Q::DesktopWallpaperDialog::DesktopWallpaperDialog(QWidget *parent) : QFileDialog(parent, "Set desktop background", QStandardPaths::writableLocation(QStandardPaths::PicturesLocation))
{
    connect(this, SIGNAL(fileSelected(const QString &)), this, SLOT(fileSelected(const QString &)));
};

void Q::DesktopWallpaperDialog::fileSelected(const QString &file)
{
    qDebug() << file;
    Desktop *desktop = static_cast<Desktop *>(parentWidget());
    desktop->setBackground(file);
    desktop->shell()->save(desktop);
};

// ----------

Q::Desktop::Desktop(Shell *shell) :
QLabel(shell),
Q::Model("Q::Desktop", shell),
myDialog(new DesktopWallpaperDialog(this))
{
    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setScaledContents(true);
    
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    geometryChanged();
    populateContextMenu();
    
    connect( QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()) );
};

// slots
void Q::Desktop::geometryChanged()
{
    resize(QGuiApplication::primaryScreen()->size());
    shell()->repaintPanels();
    repaint();
};

// configurations
void Q::Desktop::load(KConfigGroup *group)
{
    setBackground(group->readEntry("Background", ""));
};

void Q::Desktop::save(KConfigGroup *group)
{
    group->writeEntry("Background", myFileName);
};

// set background
bool Q::Desktop::setBackground(const QString &fileName)
{
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
};

// events
void Q::Desktop::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.drawImage(QGuiApplication::primaryScreen()->geometry(), myImage);
    foreach (Q::Panel *p, shell()->panels()) // HACK way to make shadows under windows on top of desktop
    {
        if(p->displaysShadow())
        {
            if(p->position() == Q::PanelPosition::Top)
            {
                QLinearGradient gradient(0, p->y(), 0, p->y() + p->height() + 15);
                gradient.setColorAt(0, QColor(0,0,0,64));
                gradient.setColorAt(1, Qt::transparent);
                painter.fillRect(p->x(), p->y() + p->height(), p->width(), p->height(), gradient);
            }
            else if(p->position() == Q::PanelPosition::Bottom)
            {
                QLinearGradient gradient(0, p->y() , 0, p->y() - p->height());
                gradient.setColorAt(0, QColor(0,0,0,64));
                gradient.setColorAt(1, Qt::transparent);
                painter.fillRect(p->x(), p->y() - p->height(), p->width(), p->height(), gradient);
            }
            else if(p->position() == Q::PanelPosition::Left)
            {
                QLinearGradient gradient(p->x() + p->width() + 10, 0, p->x() + p->width(), 0);
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, QColor(0,0,0,64));
                painter.fillRect(p->x() + p->width(),p->y(), p->width(), p->height(), gradient);
            }
            else
            {
                QLinearGradient gradient(p->x() - p->width(), 0, p->x() - p->width(), 0);
                gradient.setColorAt(0, Qt::transparent);
                gradient.setColorAt(1, QColor(0,0,0,64));
                painter.fillRect(p->x() - p->width(),p->y(), p->width(), p->height(), gradient);
            }
        }
    }
};

void Q::Desktop::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::RightButton)
    {
        myContextMenu.popup(event->pos());
    }
};

// ctx menu
void Q::Desktop::populateContextMenu()
{
    myContextMenu.clear();
    
    QAction *act;
    act = new QAction(QIcon::fromTheme("preferences-desktop-wallpaper"), "Personalize");
    connect(act, SIGNAL(triggered()), myDialog, SLOT(show()));
    myContextMenu.addAction(act);
};
