#pragma once

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QProcess>
#include <QTimer>

#include "panel.h"

namespace Q {

class ShellApplication;
class Desktop;
class Model;
class Dash;
class Shell : public QWidget
{
    Q_OBJECT
public:
    Shell();
    virtual ~Shell() {}
    void setPanelsOnTop(const bool in) { panelsOnTop = in; };
    Q::Model *getModelByName(const QString& name, Model *parent = 0);
    inline Desktop *desktop() const { return myDesktop; };
    inline Dash *dash() const { return myDash; };
    inline int getStrutLeft() const { return strut_left; };
    inline int getStrutRight() const { return strut_right; };
    inline int getStrutTop() const { return strut_top; };
    inline int getStrutBottom() const { return strut_bottom; };
    inline QList<Panel*> panels() const { return myPanels; };
    inline QTimer *oneSecond() const { return myOneSecond; };
    void save(Model *m);
    void repaintPanels();
    void kcmshell5(const QString &arg);
    void reloadAll();
public slots:
    void activateLauncherMenu();
private slots:
    void calculateStruts();
private:
    // Configurations
    void saveAll();
    void loadAll();
    QMap<QString,Model *> myModels;
    // Items
    Desktop *myDesktop;
    Dash *myDash;
    // Panels
    bool panelsOnTop = false;
    QList<Panel *> myPanels;
    void addPanel(Panel *panel);
    // Struts
    int strut_left = 0,
        strut_right = 0,
        strut_top = 0,
        strut_bottom = 0;
    QString styleSheet;
    QProcess myProcess;
    QTimer *myOneSecond;
};

}
