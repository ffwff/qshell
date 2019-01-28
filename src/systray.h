#ifndef SYSTRAY_H
#define SYSTRAY_H

#include <QAbstractNativeEventFilter>
#include <X11/Xatom.h>
#include "model.h"

namespace Q
{
    
class SystrayEventFilter : public QAbstractNativeEventFilter
{
public:
    bool nativeEventFilter(const QByteArray &eventType, void *message, long *result);
};


class Systray : public QWidget, public Model
{
public:
    Systray(const QString& name, Shell *shell);
//     void load(KConfigGroup *);
protected:
//     bool nativeEvent(const QByteArray &eventType, void *message, long *result);
};

};

#endif
