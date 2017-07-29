#include "button.h"
#include "model.h"
#include "shell.h"

Q::Button::Button(const QString &name, Shell *shell) : QPushButton(), Model(name, shell), timer(new QTimer(this))
{
};

void Q::Button::load(KConfigGroup *grp)
{
    setIcon(QIcon::fromTheme(grp->readEntry("Icon")));
    setIconSize(QSize(24,24));
    procName = grp->readEntry("LabelScript", QString());
    if(procName.isEmpty())
        setText(grp->readEntry("Label"));
    clickName = grp->readEntry("Click", QString());
    if(!clickName.isEmpty())
        connect(this, &QPushButton::clicked, [this]() {
            clickProcess.startDetached(clickName);
        });
    update();
    timer->setInterval(grp->readEntry("Interval", 1000));
    connect(timer, &QTimer::timeout, [this](){ update(); });
    timer->start();
};

void Q::Button::update()
{
    if(!procName.isEmpty())
    {
        process.start(procName);
        process.waitForFinished();
        QString output(process.readAllStandardOutput());
        setText(output.trimmed());
    }
};
