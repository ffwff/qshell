#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QProcess>
#include <QLabel>
#include <X11/Xatom.h>
#include <KF5/KWindowSystem/NETWM>
#include "model.h"

namespace Q
{

class Systray : public QLabel, public Model
{
    Q_OBJECT
public:
    Systray(const QString& name, Shell *shell);
//     void load(KConfigGroup *);
private slots:
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void windowChanged(WId wid, NET::Properties properties, NET::Properties2 properties2);
private:
    QProcess stalonetray;
    WId wid = 0;
};

};

#endif
