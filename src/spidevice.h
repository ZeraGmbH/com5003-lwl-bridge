#ifndef SPIDEVICE
#define SPIDEVICE

#include <QObject>
#include <QtSpiDevice>


class cSPIDevice: public QSPIDevice
{
    Q_OBJECT

public:
    cSPIDevice(const QString & name);

    qint64 read(char * data, qint64 len);
    qint64 write(const char * data, qint64 len);
};

#endif // SPIDEVICE

