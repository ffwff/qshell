#ifndef VOLUME_H
#define VOLUME_H

#include <QPushButton>
#include <QBoxLayout>
#include <QSlider>

#include <KF5/KConfigCore/KConfigGroup>

#include <cmath>

#include "pamixer/pulseaudio.hh"
#include "pamixer/device.hh"

#include "model.h"
#include "notificationsdialog.h"

namespace Q
{

class Shell;

class Volume;
class VolumeDialog : public NotificationsDialog
{
    Q_OBJECT
public:
    VolumeDialog(Volume *volume);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
    void update();
public slots:
    void valueChanged(int value);
protected:
    void showEvent(QShowEvent *);
private:
    Volume *myVolume;
    QSlider *slider;
    QPushButton *muteButton;
};

class Volume : public QPushButton, public Model
{
    Q_OBJECT
public:
    Volume(const QString &name, Shell *shell);
    ~Volume() { dialog->deleteLater(); };
    void load(KConfigGroup *grp) override;
    inline Pulseaudio *pulse() { return &myPulse; };
    inline Device *device() { return &myDevice; };
    inline int volumePercent() { return myDevice.volume_percent; };
    inline bool isMute() { return myDevice.mute; };
    void mute() {
        myPulse.set_mute(myDevice, !myDevice.mute);
        myDevice = myPulse.get_default_sink();
    };
    void setVolume(int value) {
        myPulse.set_volume(myDevice, round((double)value * (double)PA_VOLUME_NORM / 100.0));
        myDevice = myPulse.get_default_sink();
    };
private:
    Pulseaudio myPulse;
    Device myDevice;
    VolumeDialog *dialog;
};

};

#endif
