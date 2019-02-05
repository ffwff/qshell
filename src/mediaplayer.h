#pragma once

#include <QPushButton>
#include <QBoxLayout>
#include <QSlider>
#include <QDBusInterface>
#include <QLabel>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "notificationsdialog.h"

namespace Q {

class Shell;
class MediaPlayer;
class MediaPlayerDialog : public NotificationsDialog {
    Q_OBJECT
public:
    MediaPlayerDialog(MediaPlayer *media);
public slots:
    void update() override;
private slots:
    void playPause();
    void previousTrack();
    void nextTrack();
private:
    MediaPlayer *myMedia;
    QDBusInterface *myPropertyInterface = nullptr, *myCtrlInterface = nullptr;
    QLabel *title;
    QLabel *artist;
    QSlider *slider;
    QPushButton *next, *previous, *play;
};

class MediaPlayer : public QPushButton, public Model {
    Q_OBJECT
public:
    MediaPlayer(const QString &name, Shell *shell);
    ~MediaPlayer() { dialog->deleteLater(); };
    bool showLabel() { return myShowLabel; };
    void load(KConfigGroup *grp);
private:
    MediaPlayerDialog *dialog;
    bool myShowLabel;
};

};
