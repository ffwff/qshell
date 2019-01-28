#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QTimer>
#include <QNetworkAccessManager>

#include "network.h"
#include "model.h"
#include "shell.h"
#include "panel.h"

Q::Network::Network(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell) {
    setIcon(QIcon::fromTheme("network-wired-symbolic"));

    // TODO use networkmanager-qt
    manager = new QNetworkAccessManager(this);
    connect(manager, &QNetworkAccessManager::networkAccessibleChanged,
            [this](QNetworkAccessManager::NetworkAccessibility accessibility) {
        if(accessibility == QNetworkAccessManager::Accessible) {
            setIcon(QIcon::fromTheme("network-wired-symbolic"));
            setToolTip("Internet connected.");
        } else {
            setIcon(QIcon::fromTheme("network-wired-symbolic"));
            setToolTip("Internet disconnected.");
        }
    });
};

void Q::Network::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};
