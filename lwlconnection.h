#ifndef LWLCONNECTION
#define LWLCONNECTION

#include <QObject>
#include <QByteArray>
#include <QVector>

class QTimer;
class cSPIConnection;

const quint32 lwlInputAdress = 0x0; // data from fg301 to device
const quint16 lwlInputDataLength = 63;
const quint32 lwlOutputAdress = 0x800; // data from decice to fg301
const quint16 lwlOutputDataLength = 1696; // 256 byte actual values + status, and 720 quint16

class cLWLConnection: public QObject
{
    Q_OBJECT

public:
    cLWLConnection(cSPIConnection* spiconnection);
    ~cLWLConnection();
    bool isConnected();
    QByteArray& getLWLInput();    

    void sendActualValues(QHash<QString, double*> &actualValuesHash, int& debugCount);
    void sendOscillogram(QVector<qint16> &oscillogram);
    void sendCmdRecognized(bool on);

signals:
    void connected();
    void disconnected();
    void dataAvail();
    void error(int);

private:
    cSPIConnection *m_pSPIConnection;
    QTimer *m_pLWLLoadTimer;
    QByteArray lwlInput;
    QByteArray lwlOutput;

    bool m_bconnected;
    quint8 m_nDisconnectCount;

    void setDisconnectCount();
    void writeLWLOutput(int len);


private slots:
    void readLWLInput();
    void appendChksum(int len);
};

#endif // LWLCONNECTION

