#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QDateTime>

#include "date.h"
#include "model.h"
#include "shell.h"
#include "panel.h"

Q::Date::Date(const QString &name, Q::Shell *shell) :
QLabel(), Model(name, shell),
format("hh:mm AP")
{
    timer = new QTimer(this);
    timer->setInterval(1000);
    connect(timer, SIGNAL(timeout()), this, SLOT(update()));
    timer->start();
};

void Q::Date::load(KConfigGroup *grp)
{
    format = grp->readEntry("Format", "hh:mm AP");
    update();
};

void Q::Date::update()
{
    QDateTime dateTime = QDateTime::currentDateTime();
    setText(dateTime.toString(format));
    setToolTip(dateTime.toString());
};
