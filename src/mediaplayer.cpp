#include <QWidget>
#include <QPushButton>
#include <QIcon>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDBusReply>
#include <QDebug>
#include <QString>
#include <QVariant>
#include <QUrl>

#include "mediaplayer.h"
#include "model.h"
#include "shell.h"
#include "panel.h"
#include "frame.h"
#include "icon.h"

static const int DBUS_TIMEOUT = 25;
static const QDBusMessage findPlayerMsg = QDBusMessage::createMethodCall("org.freedesktop.DBus", "/", "org.freedesktop.DBus", "ListNames");

Q::MediaPlayer::MediaPlayer(const QString &name, Q::Shell *shell)
    : QPushButton(), Model(name, shell), dialog(new MediaPlayerDialog(this)) {
    connect(shell->oneSecond(), &QTimer::timeout, dialog, &MediaPlayerDialog::update);
}

void Q::MediaPlayer::load(KConfigGroup *grp) {
    int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
    myShowLabel = grp->readEntry("ShowLabel", false);
    setIcon(iconFromSetting(grp->readEntry("Icon", "media-playback-start")));
}

// ----------

Q::MediaPlayerDialog::MediaPlayerDialog(MediaPlayer *media)
    : NotificationsDialog(media), myMedia(media) {
    frame->resize(550, 350);

    QHBoxLayout *playout = new QHBoxLayout(this);
    setLayout(playout);

    QWidget *widget = new QWidget();
    playout->addWidget(widget);

    QVBoxLayout *layout = new QVBoxLayout(widget);
    widget->setLayout(layout);

    layout->addStretch(1);

    title = new QLabel("(Not playing)", widget);
    title->setProperty("class", "titleLabel");
    title->setWordWrap(true);
    title->setStyleSheet("font-size: 24px; padding-left: 10px;");
    layout->addWidget(title);

    artist = new QLabel("", widget);
    artist->setProperty("class", "artistLabel");
    artist->setStyleSheet("padding-left: 15px;");
    layout->addWidget(artist);

    layout->addStretch(1);

    slider = new QSlider(Qt::Horizontal, widget);
    slider->setMinimum(0);
    slider->setMaximum(1);
    slider->setTracking(true);
    layout->addWidget(slider);

    QHBoxLayout *hlayout = new QHBoxLayout(widget);
    layout->addLayout(hlayout);

    hlayout->addStretch(1);

    previous = new QPushButton(QIcon::fromTheme("media-skip-backward"), "", widget);
    connect(previous, SIGNAL(clicked()), this, SLOT(previousTrack()));
    hlayout->addWidget(previous);

    play = new QPushButton(QIcon::fromTheme("media-playback-start"), "", widget);
    connect(play, SIGNAL(clicked()), this, SLOT(playPause()));
    hlayout->addWidget(play);

    next = new QPushButton(QIcon::fromTheme("media-skip-forward"), "", widget);
    connect(next, SIGNAL(clicked()), this, SLOT(nextTrack()));
    hlayout->addWidget(next);

    hlayout->addStretch(1);

}

void Q::MediaPlayerDialog::update() {
    if(!myPropertyInterface || !myPropertyInterface->isValid()) {
        QDBusMessage response = QDBusConnection::sessionBus().call(findPlayerMsg, QDBus::Block, DBUS_TIMEOUT);

        if (response.type() == QDBusMessage::ReplyMessage) {
            QList<QVariant> args = response.arguments();
            if (args.length() == 1) {
                QVariant arg = args.at(0);
                if (!arg.isNull() && arg.isValid()) {
                    QStringList runningBusEndpoints = arg.toStringList();
                    if (!runningBusEndpoints.isEmpty()) {
                        QStringList busids;
                        for (QString& id: runningBusEndpoints) {
                            if (id.startsWith("org.mpris.MediaPlayer2.")) {
                                if(myCtrlInterface) delete myCtrlInterface;
                                myPropertyInterface = new QDBusInterface(id, "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties", QDBusConnection::sessionBus(), this);
                                myCtrlInterface = new QDBusInterface(id, "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player", QDBusConnection::sessionBus(), this);
                                break;
                            }
                        }
                    }
                }
            }
        }
    } else {
        QDBusMessage reply = myPropertyInterface->call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
        QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(reply.arguments().first());
        QVariantMap elems = qdbus_cast<QVariantMap>(dbusVariant.variant().value<QDBusArgument>() );
        if(!elems.isEmpty()) {
            QString _t = elems["xesam:title"].toString();
            title->setText(_t.isEmpty() ? QUrl(elems["xesam:url"].toString()).fileName() : _t);
            artist->setText(elems["xesam:artist"].toString());
            slider->setMinimum(0);
            slider->setMaximum(elems["mpris:length"].toLongLong() * 1e-6); // 1 microsecond = 1e-6 seconds
            if(myMedia->showLabel())
                myMedia->setText(elems["xesam:title"].toString() + " - " + elems["xesam:artist"].toString());
            QDBusReply<QDBusVariant> reply = myPropertyInterface->call("Get", "org.mpris.MediaPlayer2.Player", "Position");
            if(reply.isValid())
                slider->setValue(reply.value().variant().toLongLong() * 1e-6);
            else
                qDebug() << reply.error();
            return;
        }
    }

    title->setText("(Not playing)");
    artist->setText("");
    slider->setValue(0);
    if(myMedia->showLabel()) myMedia->setText("");
}

void Q::MediaPlayerDialog::playPause() {
    if(myPropertyInterface && myPropertyInterface->isValid()) {
        myCtrlInterface->call("PlayPause");
        QDBusReply<QDBusVariant> reply = myPropertyInterface->call("Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        if(reply.isValid()) {
            if(reply.value().variant().toString() == "Paused")
                play->setIcon(QIcon::fromTheme("media-playback-start"));
            else
                play->setIcon(QIcon::fromTheme("media-playback-pause"));
        }
    }
}

void Q::MediaPlayerDialog::nextTrack() {
    if(myPropertyInterface && myPropertyInterface->isValid()) {
        myCtrlInterface->call("Next");
    }
    update();
}

void Q::MediaPlayerDialog::previousTrack() {
    if(myPropertyInterface && myPropertyInterface->isValid()) {
        myCtrlInterface->call("Previous");
    }
    update();
}
