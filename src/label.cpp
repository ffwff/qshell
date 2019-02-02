#include "model.h"
#include "shell.h"
#include "label.h"

Q::Label::Label(const QString &name, Shell *shell)
    : QLabel(), Model(name, shell), timer(new QTimer(this)) {
}

void Q::Label::load(KConfigGroup *grp) {
    procName = grp->readEntry("LabelScript", "");
    if(procName.isEmpty())
        setText(grp->readEntry("Label"));
    update();
    const int interval = grp->readEntry("Interval", 0);
    if(interval) {
        timer->setInterval(interval);
        connect(timer, &QTimer::timeout, [this](){ if(isVisible()) update(); });
        timer->start();
    }
}

void Q::Label::update() {
    if(!procName.isEmpty()) {
        process.start(procName);
        process.waitForFinished();
        QString output(process.readAllStandardOutput());
        setText(output);
    }
}

void Q::Label::showEvent(QShowEvent *) {
    update();
}
