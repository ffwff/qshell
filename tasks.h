#ifndef TASKS_H
#define TASKS_H

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QIcon>
#include <QMouseEvent>
#include <QProcess>
#include <QMenu>

#include <KF5/KConfigCore/KConfigGroup>

#include "shell.h"
#include "model.h"

namespace Q
{

class Tasks;
class Task : public QPushButton, public Model
{
    Q_OBJECT
public:
    Task(Tasks *tasks, const QString &name);
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
public slots:
    void runCommand();
    void closeAllWindows();
    void pin();
    void unpin();
protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
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
};

}

#endif
