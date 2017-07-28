#ifndef NOTIFICATIONS_DIALOG_H
#define NOTIFICATIONS_DIALOG_H

#include <QWidget>
#include "frame.h"

namespace Q
{

class NotificationsDialog;
class NotificationsDialog : public QWidget
{
    Q_OBJECT
public:
    NotificationsDialog(QWidget *button);
    void updateDialog();
    static void hideAll();
    static QList<Frame*> frames;
    static QList<NotificationsDialog*> dialogs;
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
