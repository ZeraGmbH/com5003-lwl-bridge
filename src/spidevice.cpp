#include "spidevice.h"




cSPIDevice::cSPIDevice(const QString &name)
    :QSPIDevice(name)
{
}


qint64 cSPIDevice::read(char *data, qint64 len)
{
    return readData(data, len);
}


qint64 cSPIDevice::write(const char *data, qint64 len)
{
    return writeData(data, len);
}
