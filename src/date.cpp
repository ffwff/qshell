#include <QWidget>
#include <QLabel>
#include <QIcon>
#include <QDateTime>
#include <QHBoxLayout>
#include <QTimer>

#include "date.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "notificationsdialog.h"

Q::Date::Date(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell),
    dialog(new Q::DateDialog(this)),
    format("hh:mm AP") {
    connect(shell->oneSecond(), SIGNAL(timeout()), this, SLOT(update()));
}

void Q::Date::load(KConfigGroup *grp) {
    format = grp->readEntry("Format", "hh:mm AP");
    update();
}

void Q::Date::update() {
    QDateTime dateTime = QDateTime::currentDateTime();
    setText(dateTime.toString(format));
    setToolTip(dateTime.toString());
}

// ----------

Q::DateDialog::DateDialog(Q::Date *date) : NotificationsDialog(date), myDate(date) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);

    calendar = new QCalendarWidget(this);
    layout->addWidget(calendar);

    frame->resize(500,300);
}

void Q::DateDialog::update() {
    calendar->showToday();
}
