#ifndef NOTIFICATIONS_DIALOG_H
#define NOTIFICATIONS_DIALOG_H

#include <QWidget>
#include "frame.h"

namespace Q
{

class NotificationsDialog : public QWidget
{
    Q_OBJECT
public:
    NotificationsDialog(QWidget *button);
    void updateDialog();
    static void hideAll();
public slots:
    void toggle();
protected:
    Frame *frame;
    virtual void showEvent(QShowEvent *) { updateDialog(); };
private:
    QWidget *myButton;
};

};

#endif
