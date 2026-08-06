#include "spiconnection.h"

cSPIConnection::cSPIConnection(QSPIDevice *spictrldev, QSPIDevice *spidatadev) :
    m_pSPICtrlDevice(spictrldev),
    m_pSPIDataDevice(spidatadev)
{
}

bool cSPIConnection::writeSPI(const QByteArray &Output, quint32 OutputAdress, qint32 len)
{
    QByteArray dataBA(Output);

    // we got a bytearray greater than len and only want to send len
    // but we must use qiodevice::write(qbytearray)
    // using qiodevice::write(char*, len) stops when 0 encountered

    dataBA.resize(len);
    bool ret = setDataAdress(OutputAdress, true);
    if (ret) {
        // we could send the adress information
        // let's send data now
        // also a little bit complicated
        for (int i = 0; i < len; i++) {
            QByteArray ba;
            ba.append(char(0));
            ba.append(dataBA[i]);
            m_pSPIDataDevice->write(ba);
        }
    }
    return ret;
}

bool cSPIConnection::readSPI(QByteArray &Input, quint32 InputAdress, qint32 len)
{
    bool ret = setDataAdress(InputAdress, false);
    if (ret) {
        // we could send the adress information
        // let's read data now
        // a little bit complicated
        Input.clear();
        for (int i = 0; i < len; i++) {
            QByteArray ba = m_pSPIDataDevice->read(2);
            Input.append(ba[1]);
        }
    }
    return ret;
}

bool cSPIConnection::setStatus(devStatus stat)
{
    m_ctrlBA.clear();
    m_ctrlBA.append(0x4); // adr = 0x4 -> set adress for status led
    m_ctrlBA.append(char(0));
    m_ctrlBA.append(char(0));
    m_ctrlBA.append(char(0));
    m_ctrlBA.append(stat);

    int written = m_pSPICtrlDevice->write(m_ctrlBA);
    return (written == m_ctrlBA.size());
}

bool cSPIConnection::setDataAdress(quint32 adress, bool write)
{
    m_ctrlBA.clear();
    m_ctrlBA.append(0x10); // adr = 0x10 -> set adress for spi data read/write
    if (write)
        m_ctrlBA.append(((adress >> 24) & 0xff) | 0x80); // msb adress + write bit
    else
        m_ctrlBA.append((adress >> 24) & 0xff);
    m_ctrlBA.append((adress >> 16) & 0xff);
    m_ctrlBA.append((adress >> 8) & 0xff);
    m_ctrlBA.append(adress & 0xff);

    int written = m_pSPICtrlDevice->write(m_ctrlBA);
    return (written == m_ctrlBA.size());
}
