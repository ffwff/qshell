// original code from BE::Shell's trash.cpp
#include <QLabel>
#include <QUrl>
#include <QMouseEvent>
#include <QAction>
#include <QTimer>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QDebug>

#include <KF5/KConfigCore/KConfigGroup>
#include <KF5/KIOWidgets/KDirLister>
#include <KF5/KIOWidgets/KRun>
#include <KF5/KIOCore/KIO/CopyJob>
#include <KF5/KIOCore/KIO/SimpleJob>
#include <KF5/KCoreAddons/KJobUiDelegate>

#include "panel.h"
#include "trash.h"
#include "model.h"

KDirLister *trash = 0;

Q::Trash::Trash(const QString &name, Shell *shell) : QLabel(), Q::Model(name, shell)
{
    setAcceptDrops(true);
    setAlignment(Qt::AlignCenter);
    
    if (!trash)
    {
        trash = new KDirLister;
        trash->openUrl(QUrl("trash:/"));
    }
    connect(trash, SIGNAL(clear()), this, SLOT(updateStatus()));
    connect(trash, SIGNAL(completed()), this, SLOT(updateStatus()));
    
    QAction *act;
    
    act = new QAction("Open");
    connect(act, SIGNAL(triggered()), this, SLOT(open()));
    myContextMenu.addAction(act);
    
    act = new QAction("Empty Trash");
    connect(act, SIGNAL(triggered()), this, SLOT(empty()));
    myContextMenu.addAction(act);
};

// configs
void Q::Trash::load(KConfigGroup *grp)
{
    mySize = grp->readEntry("Size", 48);
    updateStatus();
};

// events
void Q::Trash::mouseReleaseEvent(QMouseEvent *event)
{
    if(event->button() == Qt::LeftButton)
    {
        open();
    }
    else if (event->button() == Qt::RightButton)
    {
        myContextMenu.popup(getContextMenuPos());
    }
};

void Q::Trash::dragEnterEvent(QDragEnterEvent *dee)
{
    if ( dee && dee->mimeData() && dee->mimeData()->hasUrls() )
        dee->accept();
}

void Q::Trash::dropEvent(QDropEvent *de)
{
    if (de && de->mimeData() && de->mimeData()->hasUrls()) {
        QList<QUrl> urls = de->mimeData()->urls();
        if (urls.count()) {
            de->accept();
            KIO::Job* job = KIO::trash(urls);
            job->ui()->setAutoErrorHandlingEnabled(true);
        }
    }
}

// status
void Q::Trash::updateStatus()
{
    int n = trash->items(KDirLister::AllItems).count();
    if (n > 0)
        setPixmap(QIcon::fromTheme("user-trash-full").pixmap(mySize));
    else
        setPixmap(QIcon::fromTheme("user-trash").pixmap(mySize));
};

QPoint Q::Trash::getContextMenuPos()
{
    QPoint p = parentWidget()->pos();
    QBoxLayout::Direction dir = static_cast<QBoxLayout*>(parentWidget()->layout())->direction();
    if(dir == QBoxLayout::LeftToRight || dir == QBoxLayout::RightToLeft)
    {
        p.setX(p.x() + x());
        p.setY(p.y() - myContextMenu.sizeHint().height());
    }
    else
    {
        p.setX(p.x() + parentWidget()->width());
        p.setY(p.y() + y() + myContextMenu.sizeHint().height());
    }
    return p;
};

// interactions
void Q::Trash::open()
{
    KRun::runUrl(QUrl("trash:/"), "inode/directory", 0);
};

void Q::Trash::empty()
{
    QByteArray args;
    QDataStream stream( &args, QIODevice::WriteOnly ); stream << (int)1;
    KIO::Job* job = KIO::special( QUrl("trash:/"), args );
    QTimer::singleShot(150, this, SLOT(updateStatus()));
    connect (job, SIGNAL(finished(KJob*)), this, SLOT(updateStatus()));
    trash->updateDirectory(trash->url());
};
