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

Q::Date::Date(const QString &name, Q::Shell *shell) :
QLabel(), Model(name, shell),
myDateDialog(new Q::DateDialog(this)),
format("hh:mm AP")
{
    connect(shell->oneSecond(), SIGNAL(timeout()), this, SLOT(update()));
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

void Q::Date::mouseReleaseEvent(QMouseEvent *)
{
    myDateDialog->updateDialog();
    myDateDialog->toggle();
};

// ----------

Q::DateDialog::DateDialog(Q::Date *date) : NotificationsDialog(date), myDate(date) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    setLayout(layout);
    layout->setMargin(20);

    calendar = new QCalendarWidget(this);
    layout->addWidget(calendar);

    frame->resize(500,300);
};
