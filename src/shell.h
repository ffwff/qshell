#pragma once

#include <QWidget>
#include <QList>
#include <QMap>
#include <QPixmap>
#include <QProcess>
#include <QTimer>

#include <KF5/KConfigCore/KSharedConfig>

#include "panel.h"

namespace Q {

class ShellApplication;
class Desktop;
class Model;
class Dash;
class Shell : public QWidget {
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
public slots:
    void activateLauncherMenu();
    void loadAll(const QString &file="qshellrc");
    void reloadAll(const QString &file="qshellrc");
    void showWidget(const QString &model);
    void hideWidget(const QString &model);
    void toggleWidget(const QString &model);
private slots:
    void calculateStruts();
private:
    // Configurations
    KSharedConfig::Ptr sharedConfig;
    void saveAll();
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
