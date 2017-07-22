#include <QWidget>
#include <QPainter>
#include <QDebug>

#include "shell.h"
#include "frame.h"

QT_BEGIN_NAMESPACE
  extern Q_WIDGETS_EXPORT void qt_blurImage( QPainter *p, QImage &blurImage, qreal radius, bool quality, bool alphaOnly, int transposed = 0 );
QT_END_NAMESPACE

Q::Frame::Frame(QWidget *parent) :
QWidget(parent), blurRadius(30)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
};

void Q::Frame::_showEvent()
{
    cachedShot = Q::Shell::screenshot();
};

void Q::Frame::_paintEvent()
{
    if(blurRadius)
    {
        QPainter painter(this);
        QImage img = cachedShot.toImage();
        QPixmap pxDst(img.size());
        pxDst.fill(Qt::transparent);
        QPainter p(&pxDst);
        qt_blurImage( &p, img, blurRadius, false, false );
        painter.fillRect(rect(), QColor(0,0,0,180));
        painter.drawPixmap(-x(),-y(),pxDst);
    }
};

void Q::Frame::_moveEvent()
{
    cachedShot = Q::Shell::screenshot();
};
