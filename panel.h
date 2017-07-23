#include <QWidget>
#include <QBoxLayout>
#include <QPoint>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"

#ifndef PANEL_H
#define PANEL_H

namespace Q
{

enum PanelPosition { Top = 0, Left, Bottom, Right };

class Shell;
class PanelShadow : public QWidget
{
public:
    PanelShadow(QWidget *parent = 0);
};
class Panel : public QWidget, public Model
{
    Q_OBJECT
public:
    Panel(const QString& name, Shell *shell);
    inline QBoxLayout *boxLayout() const { return static_cast<QBoxLayout*>(layout()); };
    void addWidget(QWidget *widget);
    void addStretch(int stretch = 0);
    inline PanelPosition position() const { return myPosition; };
    inline const QPoint& point() const { return myPoint; };
    inline const bool struts() const { return setStruts; };
    inline const QString& name() const { return myName; };
    inline const bool displaysShadow() const { return displayShadow; };
    inline int iconSize() { return myIconSize; };
    void load(KConfigGroup *grp) override;
    virtual void paintEvent(QPaintEvent *);
public slots:
    void geometryChanged();
private:
    Shell *myShell;
    QString myName;
    QString myWidth, myHeight;
    PanelPosition myPosition;
    QPoint myPoint;
    bool setStruts;
    int blurRadius;
    bool displayShadow;
    int myIconSize;
    float offsetTop, offsetLeft, offsetRight, offsetBottom;
};

}

#endif
