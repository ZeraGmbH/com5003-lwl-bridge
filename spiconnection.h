#ifndef SPICONNECTION
#define SPICONNECTION

#include <QByteArray>

class QSPIDevice;

class cSPIConnection
{
public:
    cSPIConnection(QSPIDevice *spictrldev, QSPIDevice *spidatadev);
    ~cSPIConnection(){}

    bool writeSPI(QByteArray& Output, quint32 OutputAdress, quint32 len);
    bool readSPI(QByteArray& Input, quint32 InputAdress, quint32 len);

private:
    QSPIDevice *m_pSPICtrlDevice;
    QSPIDevice *m_pSPIDataDevice;
    QByteArray ctrlBA;

    bool setDataAdress(quint32 adress, bool write);
};

#endif // SPICONNECTION

