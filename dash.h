#ifndef DASH_H
#define DASH_H

#include <QLineEdit>
#include <QWidget>
#include <QList>
#include <QBoxLayout>
#include <QPushButton>
#include <QProcess>

#include <KF5/KService/KServiceGroup>

#include "model.h"
#include "frame.h"

namespace Q
{
    
class DashLabelContainer : public QWidget
{
    Q_OBJECT
public:
    DashLabelContainer(QWidget *parent = 0);
};

class DashAppsContainer : public QWidget
{
    Q_OBJECT
public:
    DashAppsContainer(QWidget *parent = 0);
};

class Dash;
class DashItem : public QPushButton
{
    Q_OBJECT
public:
    DashItem(QWidget *parent, QString name, QIcon icon, QString command, QString tooltip, bool isTerminal, Dash *dash);
    void load(KConfigGroup *grp);
    void runCommand();
    void setSize(int size);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    QString myCommand;
    QStringList myArguments;
    QSize mySize;
    Dash *myDash;
    QProcess myProcess;
};

class Shell;
enum DashPosition { TopLeft = 0, TopRight, BottomLeft, BottomRight };
class Dash : public Frame, public Model
{
    Q_OBJECT
public:
    Dash(Shell *parent);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
    void load(KConfigGroup *grp) override;
    void save(KConfigGroup *grp) override;
protected:
    void showEvent(QShowEvent *);
private slots:
    void setSearch(QString s);
    void slotRepopulate();
private:
    int iconSize;
    QList<DashItem *> items;
    QWidget *searchBarContainer;
    QString search;
    QLineEdit *searchBar;
    QWidget *appsContainer;
    inline QBoxLayout *appsLayout() { return static_cast<QBoxLayout*>(appsContainer->layout()); };
    bool repopulate(KServiceGroup::Ptr group, QHBoxLayout *layout = 0, const QString &filter = 0);
    DashPosition myPosition;
    QPixmap pixmap;
    float myWidth, myHeight;
    int mySlidePosition;
};

class DashButton : public QPushButton, public Model
{
    Q_OBJECT
public:
    DashButton(const QString &name, Shell *parent);
    void load(KConfigGroup *grp) override;
protected:
    void mouseReleaseEvent(QMouseEvent *mouseEvent);
private:
    QSize mySize;
};

}

#endif
