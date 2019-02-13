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
#include "position.h"

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

class PanelStretch : public QWidget {
    Q_OBJECT
public:
    PanelStretch(QWidget *parent, Panel *panel);
protected:
    void resizeEvent(QResizeEvent *) override;
private:
    Panel *myPanel;
};

class Shell;
class Panel : public QWidget, public Model {
    Q_OBJECT
public:
    Panel(const QString& name, Shell *shell);
    void addWidget(QWidget *widget);
    void addStretch(int stretch = 0);
    inline Position position() const { return myPosition; };
    inline bool struts() const { return setStruts; };
    inline const QString &name() const { return myName; };
    inline bool displaysShadow() const { return displayShadow; };
    inline int iconSize() const { return myIconSize; };
    void load(KConfigGroup *grp) override;
    void refreshMask();
protected:
    void showEvent(QShowEvent *) override;
public slots:
    void geometryChanged();
private:
    Shell *myShell;
    QString myName;
    QString myWidth, myHeight;
    Position myPosition, mySlidePosition;
    int slideDuration;
    bool visibleByDefault;
    bool setStruts;
    int borderRadius;
    void roundCorners();
    bool transparent;
    bool stretchMask;
    void refresh();
    bool displayShadow;
    int myIconSize;
    QString offsetTop, offsetLeft, offsetRight, offsetBottom;
  	bool alwaysTop, alwaysBottom;
    PanelContainer *container;
};

}
