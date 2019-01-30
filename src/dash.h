#pragma once

#include <QLineEdit>
#include <QWidget>
#include <QList>
#include <QBoxLayout>
#include <QPushButton>
#include <QProcess>
#include <QLabel>

#include <KF5/KService/KServiceGroup>

#include "model.h"
#include "frame.h"

namespace Q {

class DashLabelContainer : public QWidget {
    Q_OBJECT
public:
    DashLabelContainer(QWidget *parent = 0);
};

class DashAppsContainer : public QWidget {
    Q_OBJECT
public:
    DashAppsContainer(QWidget *parent = 0);
};

class Dash;
class DashItem : public QPushButton {
    Q_OBJECT
public:
    DashItem(QWidget *parent, const QString &name, const QIcon &icon,
             const QString &command, const QString &tooltip,
             const bool isTerminal, Dash* dash);
    void load(KConfigGroup *grp);
    void runCommand();
    inline const QSize& size() const { return mySize; };
    void setSize(int size);
protected:
    void mouseReleaseEvent(QMouseEvent *event) override;
private:
    void updatePixmapLabels();
    QIcon icon;
    QLabel *iconLabel;
    QString myCommand;
    QStringList myArguments;
    QSize mySize;
    Dash *myDash;
    QProcess myProcess;
};

class Shell;
enum DashPosition { TopLeft = 0, TopRight, BottomLeft, BottomRight };
class Dash : public Frame, public Model {
    Q_OBJECT
public:
    Dash(Shell *parent);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
    void load(KConfigGroup *grp) override;
    void save(KConfigGroup *grp) override;
    QWidget *searchBarContainer() const { return mySearchBarContainer; }
    QWidget *searchBar() const { return mySearchBar; }
protected:
    void activeWindowChanged(WId wid);
    void showEvent(QShowEvent *);
private slots:
    void setSearch(const QString &s);
    void slotRepopulate();
private:
    int iconSize;
    QList<DashItem *> items;
    QString search;
    QWidget *mySearchBarContainer;
    QLineEdit *mySearchBar;
    QWidget *appsContainer;
    inline QLayout *appsLayout() const { return static_cast<QLayout*>(appsContainer->layout()); };
    bool repopulate(KServiceGroup::Ptr group, QLayout *layout = 0, const QString &filter = 0);
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
    QSize size;
};

}
