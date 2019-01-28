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

Q::MediaPlayer::MediaPlayer(const QString &name, Q::Shell *shell) :
QPushButton(), Model(name, shell),
dialog(new MediaPlayerDialog(this))
{
    setIcon(QIcon::fromTheme("media-playback-start"));
    connect(shell->oneSecond(), &QTimer::timeout, [this](){ dialog->update(); });
};

void Q::MediaPlayer::load(KConfigGroup *grp)
{
    myShowLabel = grp->readEntry("ShowLabel", false);
};

// ----------

Q::MediaPlayerDialog::MediaPlayerDialog(MediaPlayer *media)
    : NotificationsDialog(media), myMedia(media)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);

    layout->addStretch(1);

    title = new QLabel("(Not playing)");
    title->setWordWrap(true);
    title->setStyleSheet("font-size: 24px; padding-left: 10px;");
    layout->addWidget(title);

    artist = new QLabel("");
    artist->setStyleSheet("padding-left: 15px;");
    layout->addWidget(artist);

    layout->addStretch(1);

    slider = new QSlider(Qt::Horizontal, this);
    slider->setMinimum(0);
    slider->setMaximum(1);
    slider->setTracking(true);
    layout->addWidget(slider);

    QHBoxLayout *hlayout = new QHBoxLayout();
    layout->addLayout(hlayout);

    hlayout->addStretch(1);

    previous = new QPushButton(QIcon::fromTheme("media-skip-backward"), "");
    connect(previous, SIGNAL(clicked()), this, SLOT(previousTrack()));
    hlayout->addWidget(previous);

    play = new QPushButton(QIcon::fromTheme("media-playback-start"), "");
    connect(play, SIGNAL(clicked()), this, SLOT(playPause()));
    hlayout->addWidget(play);

    next = new QPushButton(QIcon::fromTheme("media-skip-forward"), "");
    connect(next, SIGNAL(clicked()), this, SLOT(nextTrack()));
    hlayout->addWidget(next);

    hlayout->addStretch(1);

    frame->resize(QSize(550,350));
};

void Q::MediaPlayerDialog::update() {
    if(!myPropertyInterface || !myPropertyInterface->isValid()) {
        // TODO implement MORE INTERFACES
        myPropertyInterface = new QDBusInterface("org.mpris.MediaPlayer2.vlc", "/org/mpris/MediaPlayer2", "org.freedesktop.DBus.Properties");
        myCtrlInterface = new QDBusInterface("org.mpris.MediaPlayer2.vlc", "/org/mpris/MediaPlayer2", "org.mpris.MediaPlayer2.Player");
        myPlayer = "mpris";
        if(!myPropertyInterface->isValid())
        {
            myPropertyInterface = new QDBusInterface("org.mpris.clementine", "/", "org.freedesktop.DBus.Properties");
            myPlayer = "clementine";
        }
        if(!myPropertyInterface->isValid())
        {
            myPropertyInterface = 0;
            myCtrlInterface = 0;
            myPlayer = "";
        }
    }

    if(myPlayer == "mpris" || myPlayer == "clementine") {
        QDBusMessage reply = myPropertyInterface->call("Get", "org.mpris.MediaPlayer2.Player", "Metadata");
        QVariant v = reply.arguments().first();
        QDBusVariant dbusVariant = qvariant_cast<QDBusVariant>(v);
        QVariantMap elems = qdbus_cast<QVariantMap>(dbusVariant.variant().value<QDBusArgument>() );
        if(!elems.isEmpty())
        {
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
};

void Q::MediaPlayerDialog::playPause() {
    if(myPlayer == "mpris") {
        myCtrlInterface->call("PlayPause");
        QDBusReply<QDBusVariant> reply = myPropertyInterface->call("Get", "org.mpris.MediaPlayer2.Player", "PlaybackStatus");
        if(reply.isValid()) {
            if(reply.value().variant().toString() == "Paused")
                play->setIcon(QIcon::fromTheme("media-playback-start"));
            else
                play->setIcon(QIcon::fromTheme("media-playback-pause"));
        }
    }
};

void Q::MediaPlayerDialog::nextTrack() {
    if(myPlayer == "mpris") {
        myCtrlInterface->call("Next");
    }
    update();
};

void Q::MediaPlayerDialog::previousTrack() {
    if(myPlayer == "mpris") {
        myCtrlInterface->call("Previous");
    }
    update();
};
