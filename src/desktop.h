#pragma once

#include <QLabel>
#include <QWidget>
#include <QString>
#include <QImage>
#include <QMenu>
#include <QFileDialog>
#include <QPushButton>

#include "model.h"
#include "shell.h"

namespace Q {

class DesktopIcon : public QPushButton, public Model {
    Q_OBJECT
public:
    DesktopIcon(const QString& name, Shell *shell);
    inline int left() const { return myLeft; };
    inline int top() const { return myTop; };
    void save(KConfigGroup *grp);
    void load(KConfigGroup *grp);
    void runCommand();
protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
private:
    QString myName, myCommand;
    QStringList myArguments;
    QSize mySize;
    int myLeft, myTop;
    bool pinned;
    QProcess myProcess;
};

class Desktop;
class DesktopWallpaperDialog : public QFileDialog {
    Q_OBJECT
public:
    DesktopWallpaperDialog(Desktop *parent);
public slots:
    void fileSelected(const QString &file);
private:
    Desktop *myParent;
};

class DesktopShadow : public QWidget {
    Q_OBJECT
public:
    DesktopShadow(Desktop *desktop);
protected:
    void paintEvent(QPaintEvent *);
private:
    Desktop *myDesktop;
};

class Desktop : public QWidget, public Model {
    Q_OBJECT
public:
    Desktop(Shell *shell);
    ~Desktop() { delete myShadows; }
    bool setBackground(const QString &fileName);
    inline const QString& fileName() const { return myFileName; };
    inline const QImage& image() const { return myImage; };
    inline int iconSize() const {  return myIconSize; };
    inline void repaintShadows() {
        if(myShadows != nullptr)
            myShadows->repaint();
    };
    void load(KConfigGroup *group) override;
    void save(KConfigGroup *group) override;
protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *we);
    void showEvent(QShowEvent *);
public slots:
    void geometryChanged();
private:
    QString myFileName;
    QImage myImage;
    QMenu myContextMenu;
    void populateContextMenu();
    DesktopWallpaperDialog *myDialog;
    QWidget *iconContainer;
    bool showIcons;
    QList<DesktopIcon*> myIcons;
    int myIconSize;
    DesktopShadow *myShadows = nullptr;
};

};
