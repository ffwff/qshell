#ifndef SHELL_H
#define SHELL_H

#include <QApplication>
#include <QWidget>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QProcess>
#include <QTimer>
#include <QSharedPointer>

#include "panel.h"

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
    QSharedPointer<Q::Model> getModelByName(const QString& name, QSharedPointer<Model> parent = QSharedPointer<Model>());
    inline Desktop *desktop() const { return myDesktop; };
    inline Dash *dash() const { return myDash; };
    inline int getStrutLeft() const { return strut_left; };
    inline int getStrutRight() const { return strut_right; };
    inline int getStrutTop() const { return strut_top; };
    inline int getStrutBottom() const { return strut_bottom; };
    inline QList<QSharedPointer<Panel>> panels() const { return myPanels; };
    inline QTimer *oneSecond() const { return myOneSecond; };
    void save(QSharedPointer<Model> m);
    void repaintPanels();
    void kcmshell5(const QString &arg);
    void reloadAll();
public slots:
    void geometryChanged();
private:
    // Configurations
    void saveAll();
    void loadAll();
    QMap<QString,QSharedPointer<Model>> myModels;
    //Desktop
    Desktop *myDesktop;
    // Dash
    Dash *myDash;
    // Panels
    bool panelsOnTop = false;
    QList<QSharedPointer<Panel>> myPanels;
    void addPanel(QSharedPointer<Panel> panel);
    // Struts
    int strut_left,
        strut_right,
        strut_top,
        strut_bottom;
    void calculateStruts();
    //Stylesheet
    QString styleSheet;
    // Process
    QProcess myProcess;
    // timer
    QTimer *myOneSecond;
};

class ShellApplication : public QApplication
{
    Q_OBJECT
public:
    ShellApplication(int &argc, char **argv);
};

}

#endif
