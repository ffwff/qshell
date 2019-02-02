#include <QWidget>
#include <QDBusReply>
#include <QPushButton>
#include <QIcon>
#include <QTimer>

#include "network.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "icon.h"

Q::Network::Network(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell) {

    interface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    qDebug() << interface->isValid();

    connect(this, &QPushButton::clicked, [shell]() { shell->kcmshell5("kcm_networkmanagement"); });
    connect(interface, SIGNAL(StateChanged(uint)), this, SLOT(update()));
}

void Q::Network::update() {
    QDBusReply<uint> connectivityReply = interface->call("CheckConnectivity");
    if(connectivityReply.isValid()) {
        const int connectivity = connectivityReply.value();
        if(connectivity == 4) {
            setIcon(iconConnect);
            setToolTip("Internet connected.");
        } else {
            setIcon(iconDisconnect);
            setToolTip("Internet disconnected.");
        }
    } else { // TODO fallback
        setIcon(iconConnect);
        setToolTip("Internet connected.");
    }
}

void Q::Network::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
    iconConnect = iconFromSetting(grp->readEntry("IconConnect", "network-connect"));
    iconDisconnect = iconFromSetting(grp->readEntry("IconDisconnect", "network-disconnect"));
    update();
}
