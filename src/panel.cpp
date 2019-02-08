#include <QWidget>
#include <QBoxLayout>
#include <QDebug>
#include <QApplication>
#include <QSize>
#include <QScreen>
#include <QPainter>
#include <QColor>
#include <cstring>

#include <KF5/KWindowSystem/KWindowSystem>

#include <X11/extensions/shape.h>

#include "model.h"
#include "shell.h"
#include "panel.h"
#include "desktop.h"
#include "utils.h"


Q::PanelContainer::PanelContainer(Panel *panel) : QWidget(panel), myPanel(panel) {
    QBoxLayout *layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
    layout->setSpacing(0);
    layout->setMargin(0);
    layout->setAlignment(Qt::AlignCenter);
    setLayout(layout);
}

void Q::PanelContainer::showEvent(QShowEvent *) {
    move(0, 0);
    resize(myPanel->size());
    setMaximumSize(myPanel->size());
    qDebug() << size();
}

// ----------

Q::Panel::Panel(const QString &name, Q::Shell *shell) : QWidget(shell), Q::Model(name, shell) {
    setObjectName(name);
    setWindowTitle(name);

    container = new PanelContainer(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    geometryChanged();

    connect( QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()) );
    connect( QGuiApplication::primaryScreen(), SIGNAL(virtualGeometryChanged(QRect)), this, SLOT(geometryChanged()) );
}

// rounded corners
void Q::Panel::roundCorners() {
    if(!borderRadius) return;

    const int width = this->width();
    const int height = this->height();
    const int dia = 2 * borderRadius;

    // do not try to round if the window would be smaller than the corners
    if(width < dia || height < dia)
        return;

    Display *display = QX11Info::display();
    Pixmap mask = XCreatePixmap(display, winId(), width, height, 1);
    // if this returns null, the mask is not drawable
    if(!mask)
        return;

    XGCValues xgcv;
    GC shape_gc = XCreateGC(display, mask, 0, &xgcv);
    if(!shape_gc) {
        XFreePixmap(display, mask);
        return;
    }

    XSetForeground(display, shape_gc, 0);
    XFillRectangle(display, mask, shape_gc, 0, 0, width, height);
    XSetForeground(display, shape_gc, 1);
    XFillArc(display, mask, shape_gc, 0, 0, dia, dia, 0, 23040);
    XFillArc(display, mask, shape_gc, width-dia-1, 0, dia, dia, 0, 23040);
    XFillArc(display, mask, shape_gc, 0, height-dia-1, dia, dia, 0, 23040);
    XFillArc(display, mask, shape_gc, width-dia-1, height-dia-1, dia, dia,
        0, 23040);
    XFillRectangle(display, mask, shape_gc, borderRadius, 0, width-dia, height);
    XFillRectangle(display, mask, shape_gc, 0, borderRadius, width, height-dia);
    XShapeCombineMask(display, winId(), ShapeBounding, 0, 0, mask, ShapeSet);
    XFreePixmap(display, mask);
    XFreeGC(display, shape_gc);
}

// Configurations
void Q::Panel::load(KConfigGroup *grp) {
    myWidth = grp->readEntry("Width", "100");
    myHeight = grp->readEntry("Height", "2");
    myPosition = (Position)grp->readEntry("Position", 0);
    mySlidePosition = (Position)grp->readEntry("SlidePosition", (int)myPosition);
    slideDuration = grp->readEntry("SlideDuration", 60);
    transparent = grp->readEntry("Transparent", false);
    displayShadow = grp->readEntry("DisplayShadow", true);
    myIconSize = grp->readEntry("IconSize", 24);
    offsetTop = grp->readEntry("OffsetTop", "0");
    offsetLeft = grp->readEntry("OffsetLeft", "0");
    offsetRight = grp->readEntry("OffsetRight", "0");
    offsetBottom = grp->readEntry("OffsetBottom", "0");
    borderRadius = grp->readEntry("BorderRadius", 0);
    setStruts = grp->readEntry("Struts", true);
    static_cast<QBoxLayout*>(container->layout())->setDirection((QBoxLayout::Direction)grp->readEntry("Direction", 0));

    alwaysTop = grp->readEntry("AlwaysTop", false);
    alwaysBottom = grp->readEntry("AlwaysBottom", false);

    // transparency
    if(transparent) {
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    // xlib windows
    if(!shell()->wmManagePanels()) { // workaround for i3, openbox...
        // stack
        if(alwaysTop) {
            XRaiseWindow(QX11Info::display(), winId());
        } else if(alwaysBottom) {
            XLowerWindow(QX11Info::display(), winId());
        }
        setWindowFlags(windowFlags() | Qt::X11BypassWindowManagerHint);
    } else {
        // stack
        if(alwaysTop) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
        } else if(alwaysBottom) {
            setWindowFlags(windowFlags() | Qt::WindowStaysOnBottomHint);
        }
        KWindowSystem::setState(winId(), NET::SkipTaskbar);
        KWindowSystem::setOnAllDesktops(winId(), true);
    }

    // widgets
    const QStringList widgets = grp->readEntry("Widgets", QStringList());
    foreach (const QString &w, widgets)
        if(w == "stretch")
            addStretch();
        else if(w.startsWith("stretch=")) {
            const int ratio = w.mid(strlen("stretch=")).toInt();
            addStretch(ratio);
        } else {
            Model *m = shell()->getModelByName(w);
            if(m) {
                QWidget *g = dynamic_cast<QWidget*>(m);
                if(g) {
                    qDebug() << "add widget" << m->name();
                    addWidget(g);
                }
            }
        }

    geometryChanged();
    visibleByDefault = grp->readEntry("Visible", true);
    if(visibleByDefault) QTimer::singleShot(0, this, &Q::Panel::show);
    else QTimer::singleShot(0, this, &Q::Panel::hide);
}

// Slots
void Q::Panel::geometryChanged() {
    container->hide();
    const QSize geometry = QGuiApplication::primaryScreen()->size();
    resize(dimFromSetting(myWidth, geometry.width()),
           dimFromSetting(myHeight, geometry.height()));

    if(myPosition == Position::Left || myPosition == Position::Top) {
        move(
            dimFromSetting(offsetLeft, geometry.width()),
            dimFromSetting(offsetTop, geometry.height())
        );
    } else if(myPosition == Position::Right) {
        move(
            geometry.width() - width() - dimFromSetting(offsetRight, geometry.width()),
            dimFromSetting(offsetTop, geometry.height())
        );
    } else {
        move(
            dimFromSetting(offsetLeft, geometry.width()),
            geometry.height() - height() - dimFromSetting(offsetBottom, geometry.height())
        );
    }
    container->show();
    roundCorners();
}

// Layout management
void Q::Panel::addWidget(QWidget *widget) {
    widget->setParent(container);
    container->layout()->addWidget(widget);
}

void Q::Panel::addStretch(int stretch) {
    QWidget *w = new QWidget(container);
    w->setProperty("class", "panel-stretch");
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    static_cast<QBoxLayout*>(container->layout())->addWidget(w, stretch);
}

// Rendering
void Q::Panel::showEvent(QShowEvent *) {
    roundCorners();

    if(transparent) {
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    Display *display = QX11Info::display();
    Atom atom = XInternAtom(display, "_KDE_SLIDE", false);

    QVarLengthArray<long, 1024> data(4);

    data[0] = 0;
    data[1] = mySlidePosition;
    data[2] = slideDuration;
    data[3] = slideDuration;

    XChangeProperty(display, winId(), atom, atom, 32, PropModeReplace,
            reinterpret_cast<unsigned char *>(data.data()), data.size());

    if(displayShadow && !visibleByDefault)
        myShell->desktop()->repaintShadows();
}
