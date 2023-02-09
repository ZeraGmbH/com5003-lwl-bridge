#include <QTcpSocket>

#include "ethparameterdelegate.h"


cETHParameterDelegate::cETHParameterDelegate(QTcpSocket *socket)
    :cETHCmdDelegate(socket)
{
}

void cETHParameterDelegate::setCmdList(QList<QString> sl)
{
    m_sCmdList = QList<QString>(sl);

    m_sCmdList.push_front(QString("*cls\n")); // we prepend a clear status command
    m_sCmdList.append(QString("*stb?\n")); // and append a status byte query
}


void cETHParameterDelegate::execute()
{
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
    for (int i = 0; i < m_sCmdList.count(); i++)
    {
        writeWithLog(m_sCmdList.at(i).toLatin1());
        m_pSocket->flush();
    }
}


void cETHParameterDelegate::receiveAnswer()
{
    while (m_pSocket->canReadLine())
    {
        QString answer;
        int rm;
        bool ok;

        answer = QString(m_pSocket->readLine());
        answer.remove('\n');
        answer.remove('+');

        rm = answer.toInt(&ok);
        if ( ((rm & 4) !=0) || !ok )
        {
        }
            // todo emit some error here


    }

    disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));

    emit finished();
}
