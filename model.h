#ifndef MODEL_H
#define MODEL_H

#include <QString>
#include <QSharedPointer>
#include <KF5/KConfigCore/KConfigGroup>
#include <QDebug>

namespace Q
{

class Shell;
class Model
{
public:
    Model(const QString& name, Shell *shell = 0) : myName(name), myShell(shell) {};
    virtual ~Model() { qDebug()<<"DELETE"<<myName;};
    virtual void save(KConfigGroup *grp) {};
    virtual void load(KConfigGroup *grp) {};
    inline QString name() const { return myName; };
    inline Shell *shell() const { return myShell; };
    void sync();
private:
    QString myName;
    Shell *myShell;
};

}

#endif
