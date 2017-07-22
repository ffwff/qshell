#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QTimer>

#include "network.h"
#include "model.h"
#include "shell.h"
#include "panel.h"

Q::Network::Network(const QString &name, Q::Shell *shell) :
QPushButton(), Model(name, shell)
{
    setIcon(QIcon::fromTheme("network-connect"));
    
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [this]() {
        
    });
};

void Q::Network::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};
