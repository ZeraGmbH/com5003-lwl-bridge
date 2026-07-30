#ifndef ETHCMDDELEGATE
#define ETHCMDDELEGATE

#include <QTcpSocket>
#include <QStringList>
#include <QByteArray>

class cETHCmdDelegate: public QObject
{
    Q_OBJECT
public:
    cETHCmdDelegate(QTcpSocket *socket);
    virtual void execute() = 0;

signals:
    void finished();

protected:
    void writeWithLog(const QByteArray &data);
    QByteArray readLineWithLog(qint64 maxlen = 0);
    QTcpSocket *m_pSocket;

protected slots:
    virtual void receiveAnswer() = 0;
};

#endif // ETHCMDDELEGATE

