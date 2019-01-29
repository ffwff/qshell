#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QThread>
#include <QProcess>
#include <QLabel>
#include <X11/Xatom.h>
#include <KF5/KWindowSystem/NETWM>
#include "model.h"

namespace Q
{

class Systray;
class SystrayThread : public QThread {
    Q_OBJECT
public:
    SystrayThread(WId wid);
    void run() override;
signals:
    void resize(const QRect &rect);
private:
    WId wid;
};

class Systray : public QWidget, public Model
{
    Q_OBJECT
public:
    Systray(const QString& name, Shell *shell);
    ~Systray();
    QWidget *widget() const { return myWidget; }
    WId wid() const { return myWid; }
    void load(KConfigGroup *) override;
private slots:
    void systrayResized(const QRect &rect);
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
private:
    QProcess stalonetray;
    QWidget *myWidget;
    SystrayThread *thread;
    WId myWid = 0;
    bool should_check = false;
};

};

#endif
