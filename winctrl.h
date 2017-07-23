#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <KF5/KWindowSystem/KWindowSystem>
#include "model.h"

#ifndef WINCTRL_H
#define WINCTRL_H

namespace Q
{

class Shell;
class WinCtrl : public QWidget, public Model
{
    Q_OBJECT
public:
    WinCtrl(const QString& name, Shell* shell);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
private slots:
    void update(WId wid = KWindowSystem::activeWindow());
private:
    QPushButton *closeBtn, *minimizeBtn, *maximizeBtn;
    QLabel *label;
};

};

#endif
