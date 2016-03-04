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
        // also a little bit complicated

        QByteArray ba(2,0);
        char *data = dataBA.data();

        for (int i = 0; i < len; i++,data++)
        {
            ba.replace(1,1,data); // pos, len, pointer
            m_pSPIDataDevice->write(ba);
        }
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
        // a little bit complicated

        QByteArray ba;
        Input.clear();

        for (int i = 0; i < len; i++)
        {
            ba = m_pSPIDataDevice->read(2);
            Input.append(ba[1]);
        }
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
