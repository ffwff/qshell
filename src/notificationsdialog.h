#pragma once

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
    virtual void update() {}
public slots:
    void toggle();
    void hideFrame(WId wid);
protected:
    Frame *frame;
    virtual void showEvent(QShowEvent *) { updateDialog(); };
private:
    QPushButton *myButton;
};

}
