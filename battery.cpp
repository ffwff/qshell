#include <QLabel>
#include <QTimer>
#include <QDebug>

#include <KF5/Solid/Solid/Battery>
#include <KF5/Solid/Solid/DeviceNotifier>
#include <KF5/Solid/Solid/Device>

#include "battery.h"
#include "model.h"
#include "shell.h"

Q::Battery::Battery(const QString &name, Shell *shell) : QPushButton(), Q::Model(name, shell)
{
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceAdded(QString)),
            this,                              SLOT(deviceAdded(QString)));
    connect(Solid::DeviceNotifier::instance(), SIGNAL(deviceRemoved(QString)),
            this,                              SLOT(deviceRemoved(QString)));
    
    connect(shell->oneSecond(), SIGNAL(timeout()), this, SLOT(update()));
};

void Q::Battery::update()
{
    int charge = countCharge();
    if (charge >= 67) {
        if(isCharging())
            setIcon(QIcon::fromTheme("battery-good-charging"));
        else
            setIcon(QIcon::fromTheme("battery-good"));
    } else if (charge >= 34) {
        if(isCharging())
            setIcon(QIcon::fromTheme("battery-medium-charging"));
        else
            setIcon(QIcon::fromTheme("battery-medium"));
    } else if (charge >= 0) {
        if(isCharging())
            setIcon(QIcon::fromTheme("battery-low-charging"));
        else
            setIcon(QIcon::fromTheme("battery-low"));
    } else {
        hide();
        return;
    }
};

void Q::Battery::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};

// device events
void Q::Battery::deviceAdded( const QString &udi )
{
    Solid::Device device(udi);
    if (Solid::Battery* battery = device.as<Solid::Battery>()) {
        connect(battery, SIGNAL(chargePercentChanged(int,QString)), this,
                SLOT(updateBatteryChargePercent(int,QString)));
        batteries[device.udi()] = battery;
    }
};

void Q::Battery::deviceRemoved( const QString &udi )
{
    batteries.remove(udi);
};


int Q::Battery::countCharge()
{
    if (!batteries.count())
        return -1;
    int charge = 0;
    for (QMap<QString, Solid::Battery*>::const_iterator it = batteries.constBegin(),
                                            end = batteries.constEnd(); it != end; ++it)
    {
        Solid::Battery *battery = *it;
        if(battery->chargeState() == Solid::Battery::Charging)
            charge += battery->chargePercent();
        else
            charge -= battery->chargePercent();
    }
    charge /= batteries.count();
    return charge;
};

bool Q::Battery::isCharging()
{
    for (QMap<QString, Solid::Battery*>::const_iterator it = batteries.constBegin(),
                                            end = batteries.constEnd(); it != end; ++it)
        if((*it)->chargeState() == Solid::Battery::Charging)
            return true;
    return false;
};
