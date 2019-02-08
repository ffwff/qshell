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
    static void clear();
    virtual void update() {}
public slots:
    void toggle();
    void hideFrame(WId wid);
protected:
    Frame *frame;
    void showEvent(QShowEvent *) override { updateDialog(); };
private:
    QPushButton *myButton;
};

}
