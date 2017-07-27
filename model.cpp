#include <QSharedPointer>
#include "model.h"
#include "shell.h"

void Q::Model::sync() {
    shell()->save(QSharedPointer<Model>(this));
};
