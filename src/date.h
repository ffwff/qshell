#ifndef DATE_H
#define DATE_H

#include <QLabel>
#include <QCalendarWidget>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "panel.h"
#include "notificationsdialog.h"

namespace Q {

class Shell;
class Date;
class DateDialog : public NotificationsDialog {
public:
    DateDialog(Date *date);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
private:
    Date *myDate;
    QCalendarWidget *calendar;
};

class Date : public QLabel, public Model {
    Q_OBJECT
public:
    Date(const QString &name, Shell *shell);
    ~Date() { myDateDialog->deleteLater(); };
    void load(KConfigGroup *grp) override;
protected:
    void mouseReleaseEvent(QMouseEvent *);
private slots:
    void update();
private:
    QString format;
    DateDialog *myDateDialog;
};

};

#endif
