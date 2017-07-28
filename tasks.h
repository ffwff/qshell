#ifndef TASKS_H
#define TASKS_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QIcon>
#include <QMouseEvent>
#include <QProcess>
#include <QMenu>
#include <QLabel>
#include <QTimer>

#include <KF5/KConfigCore/KConfigGroup>

#include "shell.h"
#include "model.h"
#include "frame.h"

namespace Q
{

class Task;
class TaskPreview;
class Tasks;

class Task : public QPushButton, public Model
{
    Q_OBJECT
public:
    Task(Tasks *tasks, const QString &name);
    inline Tasks *tasks() { return myParent; };
    void load(KConfigGroup *grp) override;
    void save(KConfigGroup *grp) override;
    // Command
    inline const QString& command() const {return myCommand; };
    void setCommand(QString command);
    // windows
    void addWindow(WId wid);
    void removeWindow(WId wid);
    void removeAllWindows();
    // size
    inline const QSize& size() const { return mySize; };
    // pinned
    inline bool isPinned() const { return pinned; };
    inline void setPinned(bool in) { pinned = in; };
    // Ctx menu
    void populateContextMenu();
    // previews
    inline TaskPreview *taskPreview() { return myTaskPreview; };
    QPoint getContextMenuPos(QWidget *widget);
public slots:
    void runCommand();
    void closeAllWindows();
    void pin();
    void unpin();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void enterEvent(QEvent *) override;
    void leaveEvent(QEvent *) override;
private:
    Tasks* myParent;
    QString myName, myCommand;
    QStringList myArguments;
    QSize mySize;
    QList<WId> myWindows;
    bool pinned;
    QProcess myProcess;
    QMenu myContextMenu, myWindowsContextMenu;
    void populateWindowsContextMenu();
    TaskPreview *myTaskPreview;
};

class WindowPreview;
class TaskPreview : public Frame
{
    Q_OBJECT
public:
    TaskPreview(Task *task);
    void addWindow(WId wid);
    void removeWindow(WId wid);
protected:
    void showEvent(QShowEvent *);
    void leaveEvent(QEvent*);
private:
    Task *myTask;
    QList<WindowPreview*> myPreviews;
    QList<WId> wids;
};

class WindowPreview : public QWidget
{
    Q_OBJECT
public:
    WindowPreview(WId id);
    inline const WId wid() const { return myWid; };
    void grabWindow();
signals:
    void pixmapChanged(QPixmap);
protected:
    void showEvent(QShowEvent*);
    void mouseReleaseEvent(QMouseEvent *event);
private slots:
    void KWinDBusScreenshotHelper(quint64 pixmapId);
private:
    WId myWid;
    QLabel *title;
    QLabel *window;
    QPixmap mPixmap;
};

class Tasks : public QWidget, public Model
{
    Q_OBJECT
public:
    Tasks(const QString& name, Shell *parent);
    inline QBoxLayout *boxLayout() const { return static_cast<QBoxLayout*>(layout()); };
    void addTask(Task *t);
    void removeTask(Task *t);
    void load(KConfigGroup *grp) override;
    void save(KConfigGroup *grp) override;
    inline int size() const { return mySize; };
    inline bool previewTasks() const { return myPreviewTasks; };
    void hideAllPreviews();
    inline QTimer *timer() const { return myTimer; };
protected:
    void enterEvent(QEvent *);
    void leaveEvent(QEvent*);
private slots:
    void windowAdded(WId wid);
    void windowRemoved(WId wid);
    void populateWindows();
private:
    QList<Task*> myTasks;
    Task *getTaskByCommand(const QString &command);
    QList<WId> myWindows;
    QString getCmdline(WId wid);
    int mySize;
    bool myPreviewTasks;
    QTimer *myTimer;
};

}

#endif
