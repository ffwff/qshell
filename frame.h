#include <QWidget>

#ifndef FRAME_H
#define FRAME_H

namespace Q
{

class Frame : public QWidget
{
public:
    Frame(QWidget *parent = 0);
protected:
    virtual void showEvent(QShowEvent *) { _showEvent(); };
    virtual void paintEvent(QPaintEvent *) { _paintEvent(); };
    virtual void moveEvent(QMoveEvent *) { _moveEvent(); };
    void _showEvent();
    void _paintEvent();
    void _moveEvent();
    inline void setBlurRadius(int b) { blurRadius = b; };
private:
    QPixmap cachedShot;
    int blurRadius;
};

};

#endif
