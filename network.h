#include <QPushButton>
#include <QTimer>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "panel.h"

namespace Q
{

class Shell;
class Network : public QPushButton, public Model
{
    Q_OBJECT
public:
    Network(const QString &name, Shell *shell);
    void load(KConfigGroup *grp) override;
private:
    QTimer *timer;
};

};
