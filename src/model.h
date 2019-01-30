#pragma once

#include <QString>
#include <QDebug>

#include <KF5/KConfigCore/KConfigGroup>

namespace Q {

class Shell;
class Model {
public:
    Model(const QString& name, Shell *shell = 0) : myName(name), myShell(shell) {
        qDebug()<<"MODEL"<<name;
    }
    virtual ~Model() {}
    virtual void save(KConfigGroup *) {}
    virtual void load(KConfigGroup *) {}
    inline QString name() const { return myName; }
    inline Shell *shell() const { return myShell; }
private:
    QString myName;
    Shell *myShell;
};

}
