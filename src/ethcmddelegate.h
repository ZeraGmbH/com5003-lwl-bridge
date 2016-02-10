#ifndef ETHCMDDELEGATE
#define ETHCMDDELEGATE

#include <QObject>
#include <QStringList>


class QTcpSocket;

class cETHCmdDelegate: public QObject
{
    Q_OBJECT

public:
    cETHCmdDelegate(){}
    virtual ~cETHCmdDelegate(){}

    virtual void setSocket(QTcpSocket * socket);
    virtual void execute() = 0;

signals:
    void finished();

protected:
    QTcpSocket *m_pSocket;

protected slots:
    virtual void receiveAnswer() = 0;
};

#endif // ETHCMDDELEGATE

