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

#include <KF5/KWindowSystem/KWindowSystem>

#include "model.h"
#include "desktop.h"
#include "panel.h"

Q::Desktop::Desktop(Shell *shell) : QLabel(static_cast<QWidget*>(shell)), Q::Model("Q::Desktop", shell)
{
    setBackgroundRole(QPalette::Base);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setScaledContents(true);
    
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_X11NetWmWindowTypeDesktop, true);
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    geometryChanged();
    
    connect( QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()) );
};

// slots
void Q::Desktop::geometryChanged()
{
    resize(QGuiApplication::primaryScreen()->size());
    repaint();
};

// configurations
void Q::Desktop::load(KConfigGroup *group)
{
    setBackground(group->readEntry("Background", ""));
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
                QLinearGradient gradient(0, p->y(), 0, p->y() + p->height() + 10);
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
