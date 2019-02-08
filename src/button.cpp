#include "button.h"
#include "model.h"
#include "shell.h"

Q::Button::Button(const QString &name, Shell *shell)
    : QPushButton(), Model(name, shell), timer(new QTimer(this)) {
}
Q::Button::~Button() {
    process.kill();
}

void Q::Button::load(KConfigGroup *grp) {
    setIcon(QIcon::fromTheme(grp->readEntry("Icon")));
    const int size = grp->readEntry("Size", 24);
    setIconSize(QSize(size, size));
    setMinimumSize(QSize(size, size));
    procName = grp->readEntry("LabelScript", QString());
    if(procName.isEmpty())
        setText(grp->readEntry("Label"));
    clickName = grp->readEntry("Click", QString());
    if(!clickName.isEmpty())
        connect(this, &QPushButton::clicked, [this]() {
            clickProcess.startDetached(clickName);
        });
    update();
    const int interval = grp->readEntry("Interval", 0);
    if(interval) {
        timer->setInterval(interval);
        connect(timer, &QTimer::timeout, [this](){ if(isVisible()) update(); });
        timer->start();
    }
}

void Q::Button::update() {
    if(!procName.isEmpty()) {
        process.start(procName);
        process.waitForFinished();
        QString output(process.readAllStandardOutput());
        setText(output.trimmed());
    }
}
