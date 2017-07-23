#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QDBusInterface>

#include "logout.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "frame.h"

Q::Logout::Logout(const QString &name, Q::Shell *shell) :
QPushButton(), Model(name, shell)
{
    setIcon(QIcon::fromTheme("system-shutdown"));
    
    interface = new QDBusInterface("org.kde.ksmserver", "/KSMServer", "org.kde.KSMServerInterface");
};

void Q::Logout::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};

void Q::Logout::mouseReleaseEvent(QMouseEvent *)
{
    interface->call("logout", 1, 3, 3);
};
