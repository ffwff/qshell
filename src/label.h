#pragma once

#include <QLabel>
#include <QProcess>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"

namespace Q {

class Shell;
class Label : public QLabel, public Model {
public:
    Label(const QString &name, Shell *shell);
    void load(KConfigGroup *grp);
    void showEvent(QShowEvent *) override;
private slots:
    void update();
private:
    QProcess process, clickProcess;
    QString procName;
    QTimer *timer;
};

}
