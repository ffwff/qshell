#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QPoint>

#include <X11/Xatom.h>
#include <QX11Info>
#include <X11/Xlib.h>
#include <fixx11h.h>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"

namespace Q {

class Panel;
class PanelContainer : public QWidget {
    Q_OBJECT
public:
    PanelContainer(Panel *panel);
    inline Panel *panel() { return myPanel; };
protected:
    void showEvent(QShowEvent *);
private:
    Panel *myPanel;
};

enum PanelPosition { Left = 0, Top, Right, Bottom };

class Shell;
class Panel : public QWidget, public Model {
    Q_OBJECT
public:
    Panel(const QString& name, Shell *shell);
    void addWidget(QWidget *widget);
    void addStretch(int stretch = 0);
    inline PanelPosition position() const { return myPosition; };
    inline const QPoint& point() const { return myPoint; };
    inline bool struts() const { return setStruts; };
    inline const QString& name() const { return myName; };
    inline bool displaysShadow() const { return displayShadow; };
    inline int iconSize() { return myIconSize; };
    void load(KConfigGroup *grp) override;
protected:
    virtual void showEvent(QShowEvent *);
    //virtual void paintEvent(QPaintEvent *);
public slots:
    void geometryChanged();
private:
    Shell *myShell;
    QString myName;
    QString myWidth, myHeight;
    PanelPosition myPosition;
    QPoint myPoint;
    bool visibleByDefault;
    bool setStruts;
    int blurRadius;
    bool transparent;
    bool displayShadow;
    int myIconSize;
    float offsetTop, offsetLeft, offsetRight, offsetBottom;
    PanelContainer *container;
};

}
