#include <QWidget>
#include "frame.h"

#ifndef NOTIFICATIONS_DIALOG_H
#define NOTIFICATIONS_DIALOG_H

namespace Q
{

class NotificationsDialog : public QWidget
{
    Q_OBJECT
public:
    NotificationsDialog(QWidget *button);
    void updateDialog();
public slots:
    void toggle();
protected:
    Frame *frame;
private:
    QWidget *myButton;
};

};

#endif
