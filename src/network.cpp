#include <QWidget>
#include <QDBusReply>
#include <QPushButton>
#include <QIcon>
#include <QTimer>

#include "network.h"
#include "model.h"
#include "shell.h"
#include "panel.h"

Q::Network::Network(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell) {
    setIcon(QIcon::fromTheme("network-connect"));

    interface = new QDBusInterface("org.freedesktop.NetworkManager", "/org/freedesktop/NetworkManager", "org.freedesktop.NetworkManager", QDBusConnection::systemBus());
    qDebug() << interface->isValid();

    connect(interface, SIGNAL(StateChanged(uint)), this, SLOT(update()));
};

void Q::Network::update() {
    QDBusReply<uint> connectivityReply = interface->call("CheckConnectivity");
    if(connectivityReply.isValid()) {
        const int connectivity = connectivityReply.value();
        if(connectivity == 4) {
            setIcon(QIcon::fromTheme("network-connect"));
            setToolTip("Internet connected.");
        } else {
            setIcon(QIcon::fromTheme("network-disconnect"));
            setToolTip("Internet disconnected.");
        }
    }
}

void Q::Network::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};
