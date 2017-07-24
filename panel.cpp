#include <QWidget>
#include <QBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QSize>
#include <QScreen>
#include <QPainter>
#include <QColor>
#include <algorithm>

#include <KF5/KWindowSystem/KWindowSystem>

#include "model.h"
#include "shell.h"
#include "panel.h"
#include "desktop.h"

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

Q::Panel::Panel(const QString& name, Q::Shell *shell) :
QWidget(static_cast<QWidget*>(shell)),
Q::Model(name, shell)
{
    setObjectName(name);
    
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
    
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    geometryChanged();
    
    connect( QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()) );
};

// Configurations
void Q::Panel::load(KConfigGroup *grp)
{
    myWidth = grp->readEntry("Width", "100");
    myHeight = grp->readEntry("Height", "2");
    myPosition = (Q::PanelPosition)grp->readEntry("Position", 2);
    blurRadius = grp->readEntry("BlurRadius", 0);
    displayShadow = grp->readEntry("DisplayShadow", true);
    myIconSize = grp->readEntry("IconSize", 24);
    offsetTop = grp->readEntry("OffsetTop",0.0);
    offsetLeft = grp->readEntry("OffsetLeft",0.0);
    offsetRight = grp->readEntry("OffsetRight",0.0);
    offsetBottom = grp->readEntry("OffsetBottom",0.0);
    static_cast<QBoxLayout*>(layout())->setDirection((QBoxLayout::Direction)grp->readEntry("Direction", 0));
    
    geometryChanged();
    
    QStringList widgets = grp->readEntry("Widgets", QStringList());
    foreach (QString w, widgets)
        if(w == "stretch")
            addStretch();
        else
        {
            Model *m = shell()->getModelByName(w);
            if(m)
            {
                QWidget *g = dynamic_cast<QWidget*>(m);
                if(g)
                {
                    qDebug() << "add widget" << m->name();
                    addWidget(g);
                }
            }
        }
};

// Slots
void Q::Panel::geometryChanged()
{
    QSize geometry = QGuiApplication::primaryScreen()->availableSize();
    QSize size;
    if(myWidth.endsWith("px"))
        size.setWidth(myWidth.replace("px","").toInt());
    else
        size.setWidth(geometry.width() * (myWidth.toFloat() / 100));
    if(myHeight.endsWith("px"))
        size.setHeight(myHeight.replace("px","").toInt());
    else
        size.setHeight(geometry.height() * (myHeight.toFloat() / 100));
    resize(size);
    
    if(myPosition == Q::PanelPosition::Left || myPosition == Q::PanelPosition::Top)
    {
        myPoint.setX(geometry.width() * (offsetLeft / 100));
        myPoint.setY(geometry.height() * (offsetTop / 100));
    }
    else if(myPosition == Q::PanelPosition::Right)
    {
        myPoint.setX(geometry.width() - width() - (geometry.width() * (offsetRight / 100)));
        myPoint.setY(geometry.height() * (offsetTop / 100));
    }
    else
    {
        myPoint.setX(geometry.width() * (offsetLeft / 100));
        myPoint.setY(geometry.height() - height() - (geometry.height() * (offsetBottom / 100)));
    }
    move(myPoint);
};

// Layout management
void Q::Panel::addWidget(QWidget *widget)
{
    widget->setParent(this);
    boxLayout()->addWidget(widget);
};

void Q::Panel::addStretch(int stretch)
{
    boxLayout()->addStretch(stretch);
};

// Rendering
void Q::Panel::paintEvent(QPaintEvent *)
{
    if(blurRadius)
    {
        QPainter painter(this);
        QImage img(shell()->desktop()->image().scaled(shell()->desktop()->size()));
        QPixmap pxDst(img.size());
        pxDst.fill(Qt::transparent);
        QPainter p(&pxDst);
        qt_blurImage( &p, img, blurRadius, false, false );
        painter.fillRect(rect(), QColor(0,0,0,180));
        painter.drawPixmap(-x(),-y(),pxDst);
    }
};

void Q::Panel::showEvent(QShowEvent *)
{
    Display *display = QX11Info::display();
    Atom atom = XInternAtom(display, "_KDE_SLIDE", false);

    QVarLengthArray<long, 1024> data(4);

    data[0] = 0;
    if(myPosition == PanelPosition::Top)
        data[1] = 1;
    else if(myPosition == PanelPosition::Left)
        data[1] = 0;
    else if(myPosition == PanelPosition::Right)
        data[1] = 2;
    else
        data[1] = 3;
    data[2] = 60;
    data[3] = 60;
    
    XChangeProperty(display, winId(), atom, atom, 32, PropModeReplace,
            reinterpret_cast<unsigned char *>(data.data()), data.size());
};
