#pragma once

#include <QProcess>
#include <QMap>
#include <QPushButton>
#include <QDBusInterface>
#include <X11/Xatom.h>
#include <KF5/KWindowSystem/NETWM>
#include "model.h"

namespace Q
{

class Systray;
class SystrayItem : public QPushButton {
    Q_OBJECT
public:
    SystrayItem(const QString &name, Systray *systray);
    void mouseReleaseEvent(QMouseEvent *) override;
public slots:
    void update();
private:
    QDBusInterface *interface, *itemInterface;
    Systray *systray;
};

class Systray : public QWidget, public Model {
    Q_OBJECT
public:
    Systray(const QString& name, Shell *shell);
    void load(KConfigGroup *) override;
    inline int iconSize() const {return myIconSize;}
private slots:
    void itemRegistered(QString str);
    void itemUnregistered(QString str);
private:
    void update();
    QProcess xembedsniproxy;
    QDBusInterface *statusNotifierWatcher, *statusNotifierGetter;
    QMap<QString, SystrayItem*> items;
    int myIconSize;
};

}
