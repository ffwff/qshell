#pragma once

#include <QMap>
#include <QPushButton>
#include <KF5/Solid/Solid/Battery>
#include "model.h"

namespace Q {

class Battery : public QPushButton, public Model {
    Q_OBJECT
public:
    Battery(const QString &name, Shell *shell);
    void load(KConfigGroup *grp) override;
private slots:
    void deviceAdded(const QString &udi);
    void deviceRemoved(const QString &udi);
    void update();
private:
    QMap<QString,Solid::Battery*> batteries;
    int countCharge();
    bool isCharging();
};

};
