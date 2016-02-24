#include "spiconnection.h"
#include "spidevice.h"


cSPIConnection::cSPIConnection(cSPIDevice *spictrldev, cSPIDevice *spidatadev)
    :m_pSPICtrlDevice(spictrldev), m_pSPIDataDevice(spidatadev)
{
}


bool cSPIConnection::writeSPI(QByteArray &Output, quint32 OutputAdress, quint32 len)
{
    bool ret;

    ret  = setDataAdress(OutputAdress, true);
    if (ret)
    {
        // we could send the adress information
        // let's send data now
        ret = (m_pSPIDataDevice->write(Output.data(), len) == len);
    }

    return ret;
}


bool cSPIConnection::readSPI(QByteArray &Input, quint32 InputAdress, quint32 len)
{
    bool ret;

    ret  = setDataAdress(InputAdress, false);
    if (ret)
    {
        // we could send the adress information
        // let's read data now
        ret = m_pSPIDataDevice->read(Input.data(), len);
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

    return (m_pSPICtrlDevice->write(ctrlBA.data(), 5) == 5);
}
