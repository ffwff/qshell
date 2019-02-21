#pragma once

#include <QWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QTimer>

#include <KF5/KWindowSystem/KWindowSystem>

#include "model.h"

namespace Q {

class WinTitle : public QLabel {
    Q_OBJECT
public:
    WinTitle(QWidget *parent = 0);
protected:
    void mouseDoubleClickEvent(QMouseEvent *) override;
    void mousePressEvent(QMouseEvent *) override;
    void mouseReleaseEvent(QMouseEvent *) override;
private:
    QMenu myContextMenu;
    void populateContextMenu();
    bool isDoubleClick;
    void doubleClick();
    void click();
    QTimer *timer;
};

class Shell;
class WinCtrl : public QWidget, public Model {
    Q_OBJECT
public:
    WinCtrl(const QString& name, Shell* shell);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
    void load(KConfigGroup *grp) override;
private slots:
    void update(WId wid = KWindowSystem::activeWindow());
private:
    QPushButton *closeBtn, *minimizeBtn, *maximizeBtn;
    QLabel *label;
};

}
