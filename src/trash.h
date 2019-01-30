#pragma once

#include <QLabel>
#include <QMenu>
#include "model.h"

namespace Q
{

class Trash : public QLabel, public Model
{
    Q_OBJECT
public:
    Trash(const QString &name, Shell *parent);
    void load(KConfigGroup *grp) override;
public slots:
    void open();
    void empty();
protected:
    void mouseReleaseEvent(QMouseEvent *event);
    void dragEnterEvent(QDragEnterEvent *dee);
    void dropEvent(QDropEvent *de);
private slots:
    void updateStatus();
private:
    int mySize;
    QMenu myContextMenu;
    QPoint getContextMenuPos();
};

};
