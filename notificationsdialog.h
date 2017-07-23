#include <QPushButton>
#include "frame.h"

#ifndef NOTIFICATIONS_DIALOG_H
#define NOTIFICATIONS_DIALOG_H

namespace Q
{

class NotificationsDialog : public QWidget
{
    Q_OBJECT
public:
    NotificationsDialog(QPushButton *button);
public slots:
    void toggle() { frame->setVisible(!frame->isVisible()); };
protected:
    Frame *frame;
    void updateDialog();
private:
    QPushButton *myButton;
};

};

#endif
