#ifndef MEDIAPLAYER_H
#define MEDIAPLAYER_H

#include <QPushButton>
#include <QBoxLayout>
#include <QSlider>
#include <QDBusInterface>
#include <QLabel>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "notificationsdialog.h"

namespace Q
{

class Shell;
class MediaPlayerDialog;
class MediaPlayer : public QPushButton, public Model
{
    Q_OBJECT
public:
    MediaPlayer(const QString &name, Shell *shell);
private:
    QScopedPointer<MediaPlayerDialog> dialog;
};

class MediaPlayerDialog : public NotificationsDialog
{
    Q_OBJECT
public:
    MediaPlayerDialog(MediaPlayer *media);
public slots:
    void update();
private slots:
    void playPause();
    void previousTrack();
    void nextTrack();
private:
    Q_DISABLE_COPY(MediaPlayerDialog)
    MediaPlayer *myMedia;
    QDBusInterface *myPropertyInterface, *myCtrlInterface;
    QString myPlayer;
    QLabel *title;
    QLabel *artist;
    QSlider *slider;
    QPushButton *next, *previous, *play;
};

};

#endif
