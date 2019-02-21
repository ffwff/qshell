#pragma once

#include <QPushButton>
#include <QBoxLayout>
#include <QSlider>

#include <KF5/KConfigCore/KConfigGroup>

#include <cmath>

#include <pulse/pulseaudio.h>
#include <pulse/volume.h>
#include <pulse/context.h>

#include "model.h"
#include "notificationsdialog.h"

namespace Q {

class Shell;

class Volume;
class VolumeDialog : public NotificationsDialog {
    Q_OBJECT
public:
    VolumeDialog(Volume *volume);
    inline QBoxLayout *boxLayout() { return static_cast<QBoxLayout*>(layout()); };
    void update() override;
public slots:
    void valueChanged(int value);
protected:
    void showEvent(QShowEvent *) override;
private:
    Volume *myVolume;
    QSlider *slider;
    bool sliderSet = false;
    QPushButton *muteButton;
};

class Volume : public QPushButton, public Model {
    Q_OBJECT
public:
    Volume(const QString &name, Shell *shell);
    ~Volume();
    void load(KConfigGroup *grp) override;
    int volumePercent() const;
    void setVolume(int percent);
    bool isMute() const;
    void mute();
    void populateSinkInfo();
public slots:
    void update();
protected:
    void wheelEvent(QWheelEvent *) override;
private slots:
    void sinkInfoChanged(const pa_sink_info *);
    void sinkChanged(const char *s);
private:

    enum {
        CONNECTING, CONNECTED, ERROR
    } state;
    pa_threaded_mainloop *mainloop;
    pa_context *context;
    int retval;
    pa_sink_info sinfo;
    void iterate(pa_operation *op);
    std::string sink;
    pa_cvolume *cvolume;
    void setupSubscription();
    void setupDefaultSink();

    VolumeDialog *dialog;
    QIcon iconMuted, iconHigh, iconMedium, iconLow;
    bool showLabel;
};

}
