#include <QSPIDevice>

#include "spiconnection.h"



cSPIConnection::cSPIConnection(QSPIDevice *spictrldev, QSPIDevice *spidatadev)
    :m_pSPICtrlDevice(spictrldev), m_pSPIDataDevice(spidatadev)
{
}


bool cSPIConnection::writeSPI(QByteArray &Output, quint32 OutputAdress, qint32 len)
{
    bool ret;
    QByteArray dataBA(Output);

    // we got a bytearray greater than len and only want to send len
    // but we must use qiodevice::write(qbytearray)
    // using qiodevice::write(char*, len) stops when 0 encountered

    dataBA.resize(len);
    ret  = setDataAdress(OutputAdress, true);
    if (ret)
    {
        // we could send the adress information
        // let's send data now
        ret = (m_pSPIDataDevice->write(dataBA) == dataBA.size());
    }

    return ret;
}


bool cSPIConnection::readSPI(QByteArray &Input, quint32 InputAdress, qint32 len)
{
    bool ret;

    ret  = setDataAdress(InputAdress, false);
    if (ret)
    {
        // we could send the adress information
        // let's read data now
        int read;
        Input = m_pSPIDataDevice->read(len);
        read = Input.size();

        ret = (read == len);
    }

    return ret;
}


bool cSPIConnection::setDataAdress(quint32 adress, bool write)
{
    ctrlBA.clear();
    ctrlBA.append(0x10); // adr = 0x10 -> set adress for spi data read/write
    if (write)
        ctrlBA.append(((adress >> 24) & 0xff) | 0x80); // msb adress + write bit
    else
        ctrlBA.append((adress >> 24) & 0xff);
    ctrlBA.append((adress >> 16) & 0xff);
    ctrlBA.append((adress >> 8) & 0xff);
    ctrlBA.append(adress & 0xff);

    int written;

    written = m_pSPICtrlDevice->write(ctrlBA);
    return (written == ctrlBA.size());
}
