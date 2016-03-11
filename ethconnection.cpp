#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTimer>
#include <QString>
#include <QDebug>

#include "bridge.h"
#include "ethconnection.h"
#include "bridgeconfigdata.h"

cETHConnection::cETHConnection(cBridgeConfigData *configdata)
    :m_pConfigData(configdata)
{
    m_pSocket = new QTcpSocket(this);
    connect(m_pSocket, SIGNAL(connected()), this, SLOT(regConnection()));
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(regDisconnection()));
    connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(regError(QAbstractSocket::SocketError)));
    try2Connect();
}


cETHConnection::~cETHConnection()
{
    delete m_pSocket;
}


QTcpSocket *cETHConnection::getSocket()
{
    return m_pSocket;
}


void cETHConnection::try2Connect()
{
#ifdef DEBUG
    qDebug() << QString("Bridge start connection to %1:%2").arg(m_pConfigData->m_sIPReferenceMeter).arg(m_pConfigData->m_nPortReferenceMeter);
#endif
    m_pSocket->connectToHost(QHostAddress(m_pConfigData->m_sIPReferenceMeter), m_pConfigData->m_nPortReferenceMeter);
}


void cETHConnection::regConnection()
{
    emit connected();
}


void cETHConnection::regDisconnection()
{
    try2Connect();
    emit disconnected();
}


void cETHConnection::regError(QAbstractSocket::SocketError err)
{
    qDebug() << QString("Bridge ethernet connection error %1").arg(err);

    QAbstractSocket::SocketState sockState;
    sockState = m_pSocket->state();
    if ((sockState != QAbstractSocket::ConnectingState) && (sockState != QAbstractSocket::ConnectedState))
        try2Connect();
}
