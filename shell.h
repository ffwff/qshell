#include <QApplication>
#include <QWidget>
#include <QList>
#include <QMap>
#include <QPixmap>

#include "panel.h"

#ifndef SHELL_H
#define SHELL_H

namespace Q
{

class ShellApplication;
class Desktop;
class Model;
class Dash;
class Shell : public QWidget
{
    Q_OBJECT
public:
    Shell();
    void setPanelsOnTop(bool in) { panelsOnTop = in; };
    Q::Model *getModelByName(const QString& name, Model *parent = 0);
    static QPixmap screenshot();
    inline Desktop *desktop() const { return myDesktop; };
    inline Dash *dash() const { return myDash; };
    inline int getStrutLeft() const { return strut_left; };
    inline int getStrutRight() const { return strut_right; };
    inline int getStrutTop() const { return strut_top; };
    inline int getStrutBottom() const { return strut_bottom; };
    inline QList<Panel*> panels() const { return myPanels; };
    void save(Model *m);
public slots:
    void geometryChanged();
private:
    // Configurations
    void saveAll();
    void loadAll();
    QMap<QString,Model *> myModels;
    //Desktop
    Desktop *myDesktop;
    // Dash
    Dash *myDash;
    // Panels
    bool panelsOnTop = false;
    QList<Panel *> myPanels;
    void addPanel(Panel *panel);
    // Struts
    int strut_left,
        strut_right,
        strut_top,
        strut_bottom;
    void calculateStruts();
    //Stylesheet
    QString styleSheet;
};

class ShellApplication : public QApplication
{
    Q_OBJECT
public:
    ShellApplication(int &argc, char **argv);
};

}

#endif
