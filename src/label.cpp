#include <signal.h>
#include "model.h"
#include "shell.h"
#include "label.h"

Q::Label::Label(const QString &name, Shell *shell)
    : QLabel(), Model(name, shell) {
}
Q::Label::~Label() {
    process.kill();
}

void Q::Label::load(KConfigGroup *grp) {
    procName = grp->readEntry("LabelScript", "");
    if(procName.isEmpty()) setText(grp->readEntry("Label"));
    isFifo = grp->readEntry("Fifo", false);
    if(isFifo) {
        process.start(procName);
        connect(&process, &QProcess::readyReadStandardOutput, [this]() {
            output += QString(process.readAllStandardOutput());
        });
        connect(&process, &QProcess::readyReadStandardError, [this]() {
            QString err = process.readAllStandardError();
            if(err == "[flush]") {
                setText(output);
                output = "";
            }
        });
    } else {
        update();
        const int interval = grp->readEntry("Interval", 0);
        if(interval) {
            timer = new QTimer(this);
            timer->setInterval(interval);
            connect(timer, &QTimer::timeout, [this](){ if(isVisible()) update(); });
            timer->start();
        }
    }
}

void Q::Label::update() {
    if(!isFifo && !procName.isEmpty()) {
        process.start(procName);
        process.waitForFinished();
        QString output(process.readAllStandardOutput());
        setText(output);
    }
}

void Q::Label::showEvent(QShowEvent *) {
    update();
}
