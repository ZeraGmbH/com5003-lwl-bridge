#ifndef ETHCONNECTION
#define ETHCONNECTION

#include <QObject>
#include <QAbstractSocket>


class cBridgeConfigData;
class QTimer;
class QTcpSocket;


class cETHConnection: public QObject
{
    Q_OBJECT

public:
    cETHConnection(cBridgeConfigData* configdata);
    ~cETHConnection();

    QTcpSocket* getSocket();

signals:
    void connected();
    void disconnected();
    void error(int);

private:
    cBridgeConfigData *m_pConfigData;
    QTimer *m_pConnectTimer;
    QTcpSocket *m_pSocket;

private slots:
    void try2Connect();
    void regConnection();
    void regDisconnection();
    void regError(QAbstractSocket::SocketError err);
};

#endif // ETHCONNECTION

