#ifndef NOTIFICATIONS_DIALOG_H
#define NOTIFICATIONS_DIALOG_H

#include <QWidget>
#include <QPushButton>
#include "frame.h"

namespace Q
{

class NotificationsDialog : public QWidget
{
    Q_OBJECT
public:
    NotificationsDialog(QPushButton *button);
    void updateDialog();
    static void hideAll();
public slots:
    void toggle();
    void hideFrame(WId wid);
//     void hideFrame();
protected:
    Frame *frame;
    virtual void showEvent(QShowEvent *) { updateDialog(); };
private:
    QPushButton *myButton;
};

};

#endif
