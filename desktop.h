#ifndef DESKTOP_H
#define DESKTOP_H

#include <QLabel>
#include <QWidget>
#include <QString>
#include <QImage>
#include <QMenu>
#include <QFileDialog>

#include "model.h"
#include "shell.h"

namespace Q
{

class DesktopWallpaperDialog : public QFileDialog
{
    Q_OBJECT
public:
    DesktopWallpaperDialog(QWidget *parent = 0);
public slots:
    void fileSelected(const QString &file);
};

class Desktop : public QLabel, public Model
{
    Q_OBJECT
public:
    Desktop(Shell *shell);
    bool setBackground(const QString &fileName);
    inline const QString& fileName() const { return myFileName; };
    inline const QImage& image() const { return myImage; };
    void load(KConfigGroup *group) override;
    void save(KConfigGroup *group) override;
protected:
    void paintEvent(QPaintEvent *);
    void mouseReleaseEvent(QMouseEvent *event);
public slots:
    void geometryChanged();
private:
    QString myFileName;
    QImage myImage;
    QMenu myContextMenu;
    void populateContextMenu();
    DesktopWallpaperDialog *myDialog;
};

};

#endif
