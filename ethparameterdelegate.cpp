#include <QTcpSocket>

#include "ethparameterdelegate.h"


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
        m_pSocket->write(m_sCmdList.at(i).toLatin1());
}


void cETHParameterDelegate::receiveAnswer()
{
    while (m_pSocket->canReadLine())
    {
        QString answer;
        int rm;
        bool ok;

        answer = QString(m_pSocket->readLine());
        rm = answer.toInt(&ok);
        if ( ((rm & 4) !=0) || !ok )
        {
        }
            // todo emit some error here


    }

    emit finished();
    disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
}
