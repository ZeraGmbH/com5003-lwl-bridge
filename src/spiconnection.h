#ifndef SPICONNECTION
#define SPICONNECTION

#include <QByteArray>

class cSPIDevice;

class cSPIConnection
{
public:
    cSPIConnection(cSPIDevice *spictrldev, cSPIDevice *spidatadev);
    ~cSPIConnection(){}

    bool writeSPI(QByteArray& Output, quint32 OutputAdress, quint32 len);
    bool readSPI(QByteArray& Input, quint32 InputAdress, quint32 len);

private:
    cSPIDevice *m_pSPICtrlDevice;
    cSPIDevice *m_pSPIDataDevice;
    QByteArray ctrlBA;

    bool setDataAdress(quint32 adress, bool write);
};

#endif // SPICONNECTION

