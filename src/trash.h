#pragma once

#include <QLabel>
#include <QMenu>
#include <QIcon>
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
    void mouseReleaseEvent(QMouseEvent *) override;
    void dragEnterEvent(QDragEnterEvent *) override;
    void dropEvent(QDropEvent *) override;
private slots:
    void updateStatus();
private:
    int mySize;
    QIcon iconFull, iconEmpty;
    QMenu myContextMenu;
    QPoint getContextMenuPos();
};

}
