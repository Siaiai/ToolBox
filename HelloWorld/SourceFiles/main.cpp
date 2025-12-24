#include "HelloWorld.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    HelloWorld window;
    window.show();
    return app.exec();
}
