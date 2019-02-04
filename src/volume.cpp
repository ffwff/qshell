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
#include "utils.h"

Q::Volume::Volume(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell),
    myPulse(Pulseaudio("qshell")),
    myDevice(myPulse.get_default_sink()),
    dialog(new Q::VolumeDialog(this)) {
    connect(shell->oneSecond(), &QTimer::timeout, this, &Q::Volume::update);
}

void Q::Volume::update() {
    myDevice = myPulse.get_default_sink();
    if(isMute()) {
        setIcon(iconMuted);
        setToolTip("Muted");
    } else {
        if(volumePercent() > 60)
            setIcon(iconHigh);
        else if(volumePercent() > 25)
            setIcon(iconMedium);
        else
            setIcon(iconLow);
        setToolTip(QString::number(volumePercent()) + "%");
    }
    if(dialog->isVisible())
        dialog->update();
}

void Q::Volume::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
    iconMuted  = iconFromSetting(grp->readEntry("IconMuted", "audio-volume-muted"));
    iconHigh   = iconFromSetting(grp->readEntry("IconHigh", "audio-volume-high"));
    iconMedium = iconFromSetting(grp->readEntry("IconMedium", "audio-volume-medium"));
    iconLow    = iconFromSetting(grp->readEntry("IconLow", "audio-volume-low"));
    update();
}

void Q::Volume::wheelEvent(QWheelEvent *we) {
    setVolume(volumePercent()+( we->delta()>0 ? 1 : -1 ));
    update();
}

// ----------

Q::VolumeDialog::VolumeDialog(Volume *volume) :
    Q::NotificationsDialog(volume),
    myVolume(volume) {
    frame->resize(QSize(280,40));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setSpacing(0);
    setLayout(layout);

    muteButton = new QPushButton(QIcon::fromTheme("audio-volume-muted"), "Mute", this);
    muteButton->setFixedSize(100, muteButton->height());
    layout->addWidget(muteButton);
    connect(muteButton, &QPushButton::clicked, [this]() {
        myVolume->mute();
        update();
    });


    slider = new QSlider(Qt::Horizontal, this);
    slider->setMinimum(0);
    slider->setMaximum(200);
    slider->setTracking(true);
    layout->addWidget(slider, 1);
    slider->setMinimumSize(slider->width(), muteButton->height());
    connect(slider, SIGNAL(valueChanged(int)), this, SLOT(valueChanged(int)));
}

void Q::VolumeDialog::update() {
    if(myVolume->isMute()) {
        muteButton->setText("Unmute");
    } else {
        muteButton->setText("Mute");
    }
    slider->setValue(myVolume->volumePercent());
}

void Q::VolumeDialog::showEvent(QShowEvent *) {
    updateDialog();
    update();
}

void Q::VolumeDialog::valueChanged(int value) {
    myVolume->setVolume(value);
}
