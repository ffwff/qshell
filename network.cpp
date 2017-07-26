#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QTimer>
#include <QNetworkAccessManager>

#include "network.h"
#include "model.h"
#include "shell.h"
#include "panel.h"

Q::Network::Network(const QString &name, Q::Shell *shell) :
QPushButton(), Model(name, shell)
{
    setIcon(QIcon::fromTheme("network-connect"));
    
    manager = new QNetworkAccessManager(this);
    connect(shell->oneSecond(), &QTimer::timeout, [this]() {
        QNetworkAccessManager::NetworkAccessibility accessibility = manager->networkAccessible();
        if(accessibility == QNetworkAccessManager::Accessible)
        {
            setIcon(QIcon::fromTheme("network-connect"));
            setToolTip("Internet connected.");
        }
        else
        {
            setIcon(QIcon::fromTheme("network-disconnect"));
            setToolTip("Internet disconnected.");
        }
    });
};

void Q::Network::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};
