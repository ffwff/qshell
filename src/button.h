#pragma once

#include <QPushButton>
#include <QProcess>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"

namespace Q {

class Shell;
class Button : public QPushButton, public Model {
public:
    Button(const QString &name, Shell *shell);
    void load(KConfigGroup *grp);
private slots:
    void update();
private:
    QProcess process, clickProcess;
    QString procName, clickName;
    QTimer *timer;
};

};
