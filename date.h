#include <QLabel>
#include <QTimer>

#include <KF5/KConfigCore/KConfigGroup>

#include "model.h"
#include "panel.h"

namespace Q
{

class Shell;
class Date : public QLabel, public Model
{
    Q_OBJECT
public:
    Date(const QString &name, Shell *shell);
    void load(KConfigGroup *grp) override;
private slots:
    void update();
private:
    QTimer *timer;
    QString format;
};

};
