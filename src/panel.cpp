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

#include "model.h"
#include "shell.h"
#include "panel.h"
#include "desktop.h"

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

    container = new PanelContainer(this);

    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setAttribute(Qt::WA_X11NetWmWindowTypeDock, true);
    setAttribute(Qt::WA_AlwaysShowToolTips, true);
    geometryChanged();

    connect( QGuiApplication::primaryScreen(), SIGNAL(geometryChanged(QRect)), this, SLOT(geometryChanged()) );
    connect( QGuiApplication::primaryScreen(), SIGNAL(virtualGeometryChanged(QRect)), this, SLOT(geometryChanged()) );
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
    const QString iconTheme = grp->readEntry("IconTheme", "");
    offsetTop = grp->readEntry("OffsetTop",0.0);
    offsetLeft = grp->readEntry("OffsetLeft",0.0);
    offsetRight = grp->readEntry("OffsetRight",0.0);
    offsetBottom = grp->readEntry("OffsetBottom",0.0);
    setStruts = grp->readEntry("Struts", true);
    static_cast<QBoxLayout*>(container->layout())->setDirection((QBoxLayout::Direction)grp->readEntry("Direction", 0));

    geometryChanged();

    if(transparent) {
        setAttribute(Qt::WA_NoSystemBackground, true);
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

    KWindowSystem::setState(winId(), NET::SkipTaskbar);
    KWindowSystem::setOnAllDesktops(winId(), true);

    QStringList widgets = grp->readEntry("Widgets", QStringList());
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

    visibleByDefault = grp->readEntry("Visible", true);
    if(visibleByDefault) show();
    else hide();
}

// Slots
void Q::Panel::geometryChanged() {
    container->hide();
    QSize geometry = QGuiApplication::primaryScreen()->size();
    QSize size;
    if(myWidth.endsWith("px"))
        size.setWidth(QString(myWidth).replace("px","").toInt());
    else
        size.setWidth(geometry.width() * (myWidth.toFloat() / 100));
    if(myHeight.endsWith("px"))
        size.setHeight(QString(myHeight).replace("px","").toInt());
    else
        size.setHeight(geometry.height() * (myHeight.toFloat() / 100));
    resize(size);

    if(myPosition == Position::Left || myPosition == Position::Top) {
        myPoint.setX(geometry.width() * (offsetLeft / 100));
        myPoint.setY(geometry.height() * (offsetTop / 100));
    } else if(myPosition == Position::Right) {
        myPoint.setX(geometry.width() - width() - (geometry.width() * (offsetRight / 100)));
        myPoint.setY(geometry.height() * (offsetTop / 100));
    } else {
        myPoint.setX(geometry.width() * (offsetLeft / 100));
        myPoint.setY(geometry.height() - height() - (geometry.height() * (offsetBottom / 100)));
    }
    move(myPoint);
    container->show();
}

// Layout management
void Q::Panel::addWidget(QWidget *widget) {
    widget->setParent(container);
    container->layout()->addWidget(widget);
}

void Q::Panel::addStretch(int stretch) {
    QWidget *w = new QWidget(container);
    w->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    static_cast<QBoxLayout*>(container->layout())->addWidget(w, stretch);
}

// Rendering
void Q::Panel::showEvent(QShowEvent *) {
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
