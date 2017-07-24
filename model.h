#ifndef MODEL_H
#define MODEL_H

#include <QString>

#include <KF5/KConfigCore/KConfigGroup>

namespace Q
{

class Shell;
class Model
{
public:
    Model(const QString& name, Shell *shell = 0) : myName(name), myShell(shell) {};
    virtual ~Model() {};
    virtual void save(KConfigGroup *grp) {};
    virtual void load(KConfigGroup *grp) {};
    inline QString name() const { return myName; };
    inline Shell *shell() const { return myShell; };
private:
    QString myName;
    Shell *myShell;
};

}

#endif
