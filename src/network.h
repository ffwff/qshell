#ifndef NETWORK_H
#define NETWORK_H

#include <QPushButton>
#include <QDBusInterface>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "panel.h"

namespace Q
{

class Shell;
class Network : public QPushButton, public Model
{
    Q_OBJECT
public:
    Network(const QString &name, Shell *shell);
    void load(KConfigGroup *grp) override;
public slots:
    void update();
private:
    QDBusInterface *interface;
};

};

#endif
