#pragma once

#include <QApplication>

#include "shell.h"

namespace Q {

class ShellApplication : public QApplication {
public:
    ShellApplication(int argc, char **argv);
    ~ShellApplication() {}
private:
    Shell *myShell;
};

}
