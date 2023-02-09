#include "ethcmddelegate.h"
#include "QTcpSocket"

cETHCmdDelegate::cETHCmdDelegate(QTcpSocket *socket)
    :m_pSocket(socket)
{
}

void cETHCmdDelegate::writeWithLog(const QByteArray &data)
{
    qInfo("SCPI send: %s", qPrintable(data));
    m_pSocket->write(data);
}
