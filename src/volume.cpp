#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QHBoxLayout>
#include <QSlider>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QScreen>
#include <QWheelEvent>

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
    dialog(new Q::VolumeDialog(this)) {
    setIcon(QIcon::fromTheme("audio-volume-high"));

    connect(shell->oneSecond(), &QTimer::timeout, this, &Q::Volume::update);
};

void Q::Volume::update() {
    myDevice = myPulse.get_default_sink();
    if(isMute()) {
        setIcon(QIcon::fromTheme("audio-volume-muted"));
        setToolTip("Muted");
    } else {
        if(volumePercent() > 60)
            setIcon(QIcon::fromTheme("audio-volume-high"));
        else if(volumePercent() > 25)
            setIcon(QIcon::fromTheme("audio-volume-medium"));
        else
            setIcon(QIcon::fromTheme("audio-volume-low"));
        setToolTip(QString::number(volumePercent()) + "%");
    }
    if(dialog->isVisible())
        dialog->update();
}

void Q::Volume::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
}

void Q::Volume::wheelEvent(QWheelEvent *we) {
    setVolume(volumePercent()+( we->delta()>0 ? 1 : -1 ));
    update();
}

// ----------

Q::VolumeDialog::VolumeDialog(Volume *volume) :
    Q::NotificationsDialog(volume),
    myVolume(volume) {
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(20, 0, 20, 0);
    layout->setSpacing(0);
    setLayout(layout);

    muteButton = new QPushButton(QIcon::fromTheme("audio-volume-muted"), "Mute", this);
    layout->addWidget(muteButton);
    connect(muteButton, &QPushButton::clicked, [this]() {
        myVolume->mute();
        update();
    });

    frame->resize(QSize(280,70));

    slider = new QSlider(Qt::Horizontal, this);
    slider->setMinimum(0);
    slider->setMaximum(200);
    slider->setTracking(true);
    layout->addWidget(slider, 1);
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
};

void Q::VolumeDialog::update() {
    if(myVolume->isMute()) {
        muteButton->setText("Unmute");
    } else {
        muteButton->setText("Mute");
    }
    slider->setValue(myVolume->volumePercent());
};

void Q::VolumeDialog::showEvent(QShowEvent *) {
    updateDialog();
    update();
};

void Q::VolumeDialog::valueChanged(int value) {
    myVolume->setVolume(value);
};
