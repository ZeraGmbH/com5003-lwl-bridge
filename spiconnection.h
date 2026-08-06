#ifndef SPICONNECTION
#define SPICONNECTION

#include <QSPIDevice>
#include <QByteArray>

enum devStatus
{
    bridgeIdleStatus = 1,
    bridgeETHConnectedStatus = 2,
    bridgeLWLConnectedStatus = 3,
    bridgeActiveStatus = 7 // 7 means status led is permanent on
};

class cSPIConnection
{
public:
    cSPIConnection(QSPIDevice *spictrldev, QSPIDevice *spidatadev);
    ~cSPIConnection(){}

    bool writeSPI(const QByteArray& Output, quint32 OutputAdress, qint32 len);
    bool readSPI(QByteArray& Input, quint32 InputAdress, qint32 len);
    bool setStatus(devStatus stat);

private:
    bool setDataAdress(quint32 adress, bool write);

    QSPIDevice *m_pSPICtrlDevice = nullptr;
    QSPIDevice *m_pSPIDataDevice = nullptr;
    QByteArray m_ctrlBA;
};

#endif // SPICONNECTION

