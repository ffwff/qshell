#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QHBoxLayout>
#include <QSlider>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <algorithm>

#include <pulse/pulseaudio.h>
#include "pamixer/pulseaudio.hh"
#include "pamixer/device.hh"

#include "volume.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "frame.h"

Q::Volume::Volume(const QString &name, Q::Shell *shell) :
QPushButton(), Model(name, shell),
myPulse(Pulseaudio("qshell")),
myDevice(myPulse.get_default_sink()),
dialog(new Q::VolumeDialog(this))
{
    setIcon(QIcon::fromTheme("audio-volume-high"));
    connect(this, SIGNAL(clicked()), dialog, SLOT(toggle()));
    
    myTimer = new QTimer(this);
    myTimer->setInterval(1000);
    connect(myTimer, &QTimer::timeout, [this]() {
        if(isMute()) {
            setIcon(QIcon::fromTheme("audio-volume-muted"));
            setToolTip("Muted");
        }
        else
        {
            if(volumePercent() > 60)
                setIcon(QIcon::fromTheme("audio-volume-high"));
            else if(volumePercent() > 25)
                setIcon(QIcon::fromTheme("audio-volume-medium"));
            else
                setIcon(QIcon::fromTheme("audio-volume-low"));
            setToolTip(QString::number(volumePercent()) + "%");
        }
        
    });
    myTimer->start();
};

void Q::Volume::load(KConfigGroup *grp)
{
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
};

// ----------

Q::VolumeDialog::VolumeDialog(Volume *volume) : Q::Frame(volume->shell())
{
    myVolume = volume;
    setLayout(new QHBoxLayout());
    
    muteButton = new QPushButton(QIcon::fromTheme("audio-volume-muted"), "Mute");
    if(myVolume->isMute()) {
        muteButton->setText("Unmute");
    } else {
        muteButton->setText("Mute");
    }
    boxLayout()->addWidget(muteButton);
    connect(muteButton, &QPushButton::clicked, [this]() {
        myVolume->mute();
        if(myVolume->isMute()) {
            muteButton->setText("Unmute");
        } else {
            muteButton->setText("Mute");
        }
    });
    
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    resize(QSize(210,40));
    
    slider = new QSlider(Qt::Horizontal, this);
    slider->setMinimum(0);
    slider->setMaximum(200);
    slider->setTracking(true);
    boxLayout()->addWidget(slider);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
};

void Q::VolumeDialog::showEvent(QShowEvent *)
{
    QPoint p = myVolume->parentWidget()->pos();
    Shell *shell = static_cast<Shell*>(parentWidget());
    QScreen *screen = QGuiApplication::primaryScreen();
    
    int _x;
    if((_x = std::max(shell->getStrutLeft(), p.x() + myVolume->x() + width())) < screen->size().width())
        p.setX(_x);
    else
        p.setX(screen->size().width() - width() );
    
    int _y;
    if((_y = std::max(shell->getStrutTop(), p.y() + myVolume->y() - height())) < screen->size().height())
        p.setY(_y);
    else
        p.setY(screen->size().height() - static_cast<Shell*>(parentWidget())->getStrutBottom() - height() );
    
    move(p);
    
    slider->setValue(myVolume->volumePercent());
    _showEvent();
};

void Q::VolumeDialog::valueChanged(int value)
{
    myVolume->setVolume(value);
};
