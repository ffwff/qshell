#include <QLabel>
#include <QWidget>
#include <QString>
#include <QImage>

#include "model.h"
#include "shell.h"

#ifndef DESKTOP_H
#define DESKTOP_H

namespace Q
{

class Desktop : public QLabel, public Model
{
    Q_OBJECT
public:
    Desktop(Shell *shell);
    bool setBackground(const QString &fileName);
    inline const QString& fileName() const { return myFileName; };
    inline const QImage& image() const { return myImage; };
    virtual void paintEvent(QPaintEvent *);
    void load(KConfigGroup *group) override;
private:
    QString myFileName;
    QImage myImage;
};

};

#endif
