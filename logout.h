#include <QPushButton>
#include <QBoxLayout>
#include <QSlider>
#include <QDBusInterface>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "notificationsdialog.h"

namespace Q
{

class Shell;
class Logout : public QPushButton, public Model
{
    Q_OBJECT
public:
    Logout(const QString &name, Shell *shell);
    void load(KConfigGroup *grp) override;
protected:
    void mouseReleaseEvent(QMouseEvent *);
private:
    QDBusInterface *interface;
};

};
