#include <QCoreApplication>

#include "bridge.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    cBridge *bridge;
    bridge = new cBridge();

    return a.exec();

    delete bridge;
}
