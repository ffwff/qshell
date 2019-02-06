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

#include "volume.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "frame.h"
#include "utils.h"

static const auto success_cb = [](pa_context *, int, void *) {};

Q::Volume::Volume(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell),
    dialog(new Q::VolumeDialog(this)) {

    mainloop = pa_threaded_mainloop_new();
    pa_threaded_mainloop_lock(mainloop);
    pa_mainloop_api *api = pa_threaded_mainloop_get_api(mainloop);
    context = pa_context_new(api, "qshell");

    if (pa_threaded_mainloop_start(mainloop)) {
        qDebug() << "Unable to start pulseaudio mainloop";
        pa_threaded_mainloop_free(mainloop);
        mainloop = nullptr;
        return;
    }

    pa_context_set_state_callback(context, [](pa_context *context, void *volume_) {
        Q::Volume *volume = (Q::Volume *)volume_;
        switch(pa_context_get_state(context)) {
            case PA_CONTEXT_READY:
                volume->state = CONNECTED;
                volume->setupSubscription();
                volume->setupDefaultSink();
                break;
            case PA_CONTEXT_FAILED:
                volume->state = ERROR;
                break;
            case PA_CONTEXT_UNCONNECTED:
            case PA_CONTEXT_AUTHORIZING:
            case PA_CONTEXT_SETTING_NAME:
            case PA_CONTEXT_CONNECTING:
            case PA_CONTEXT_TERMINATED:
                break;
        }
    }, this);

    state = CONNECTING;
    if (pa_context_connect(context, NULL, PA_CONTEXT_NOFLAGS, NULL) < 0 ||
        state == ERROR) {
        qDebug() << "Connection error\n";
        return;
    }
    pa_threaded_mainloop_unlock(mainloop);
}

Q::Volume::~Volume() {
    pa_threaded_mainloop_free(mainloop);
    dialog->deleteLater();
}

// setup

void Q::Volume::setupSubscription() {
    qDebug() << "subscription";
    pa_operation *op;
    // subscribe to events
    pa_context_set_subscribe_callback(context, [](pa_context *c, pa_subscription_event_type_t t, uint32_t idx, void *volume_) {
        qDebug() << "subscribe";
        Q::Volume *volume = (Q::Volume *)volume_;
        if(t == PA_SUBSCRIPTION_EVENT_CHANGE && idx == volume->sinfo.index) {
            volume->populateSinkInfo();
        }
    }, this);
    if(!(op = pa_context_subscribe(context, PA_SUBSCRIPTION_MASK_SINK, success_cb, nullptr)))
        qDebug() << "Can't subscribe";

}

void Q::Volume::setupDefaultSink() {
    pa_operation *op = pa_context_get_server_info(context, [](pa_context *, const pa_server_info *i, void *volume_) {
        Q::Volume *volume = (Q::Volume *)volume_;
        emit volume->sinkChanged(i->default_sink_name);
    }, this);
    pa_operation_unref(op);
}

void Q::Volume::populateSinkInfo() {
    qDebug() << "populate...";
    pa_operation *op = pa_context_get_sink_info_by_name(context, sink.c_str(), [](pa_context *, const pa_sink_info *i, int eol, void *volume_) {
        if(eol != 0) return;
        Q::Volume *volume = (Q::Volume *)volume_;
        emit volume->sinkInfoChanged(i);
        emit volume->update();
    }, this);
    pa_operation_unref(op);
}

// slots
void Q::Volume::sinkInfoChanged(const pa_sink_info *i) {
    sinfo.index = i->index;
    sinfo.mute = i->mute;
    sinfo.volume = i->volume;
}

void Q::Volume::sinkChanged(const char *s) {
    if(!sink.empty()) return;
    sink = s;
    populateSinkInfo();
}

// etc
void Q::Volume::iterate(pa_operation *op) {
    while (pa_operation_get_state(op) == PA_OPERATION_RUNNING)
        pa_threaded_mainloop_wait(mainloop);
}

int Q::Volume::volumePercent() const {
    if(isMute()) return -1;
    int volume_avg = pa_cvolume_avg(&sinfo.volume);
    int percent = (int)round((double)volume_avg * 100. / PA_VOLUME_NORM);
    return percent;
}

bool Q::Volume::isMute() const {
    return (bool)sinfo.mute;
}

void Q::Volume::mute() {
    pa_operation *op = pa_context_set_sink_mute_by_name(context, sink.c_str(), (int)(!isMute()), success_cb, nullptr);
    iterate(op);
    pa_operation_unref(op);
    populateSinkInfo();
}

void Q::Volume::setVolume(int percent) {
    pa_volume_t new_volume = round((double)percent * (double)PA_VOLUME_NORM / 100.0);
    if (new_volume > PA_VOLUME_MAX) {
        new_volume = PA_VOLUME_MAX;
    }
    pa_cvolume *new_cvolume = pa_cvolume_set(&sinfo.volume, sinfo.volume.channels, new_volume);
    pa_operation *op = pa_context_set_sink_volume_by_name(context, sink.c_str(), new_cvolume, success_cb, nullptr);
    iterate(op);
    pa_operation_unref(op);
    populateSinkInfo();
}

void Q::Volume::update() {
    int percent = volumePercent();
    if(percent == -1) {
        setIcon(iconMuted);
        setToolTip("Muted");
    } else {
        if(percent > 60)
            setIcon(iconHigh);
        else if(percent > 25)
            setIcon(iconMedium);
        else
            setIcon(iconLow);
        setToolTip(QString::number(percent) + "%");
    }
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
    if(myVolume->volumePercent() == -1) {
        muteButton->setText("Unmute");
    } else {
        muteButton->setText("Mute");
    }
    sliderSet = true;
    slider->setValue(myVolume->volumePercent());
}

void Q::VolumeDialog::showEvent(QShowEvent *) {
    updateDialog();
}

void Q::VolumeDialog::valueChanged(int value) {
    if(sliderSet) {
        sliderSet = false;
        return;
    }
    myVolume->setVolume(value);
}
