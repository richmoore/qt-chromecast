#include <QCoreApplication>

#include "discoverer.h"
#include "devicelister.h"

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    Discoverer disco;
    DeviceLister lister;

    QObject::connect(&disco, SIGNAL(foundDialDevice(const QUrl &, const QByteArray &)),
                     &lister, SLOT(addDialDevice(const QUrl &, const QByteArray &)));

    disco.startDiscovery();

    return app.exec();
}
