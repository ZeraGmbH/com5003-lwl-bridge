#include <QTcpSocket>
#include <QAbstractSocket>
#include <QHostAddress>
#include <QTimer>
#include <QString>
#include <QDebug>
#include <QTimer>

#include "bridge.h"
#include "ethconnection.h"
#include "bridgeconfigdata.h"

cETHConnection::cETHConnection(cBridgeConfigData *configdata)
    :m_pConfigData(configdata)
{
    m_pRetryTimer = new QTimer();
    m_pRetryTimer->setSingleShot(true);
    connect(m_pRetryTimer, SIGNAL(timeout()), this, SLOT(try2Connect()));

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
    qInfo() << QString("Bridge start connection to %1:%2").arg(m_pConfigData->m_sIPReferenceMeter).arg(m_pConfigData->m_nPortReferenceMeter);
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
    qWarning() << QString("Bridge ethernet connection error %1").arg(err);

    QAbstractSocket::SocketState sockState;
    sockState = m_pSocket->state();
    if ((sockState != QAbstractSocket::ConnectingState) && (sockState != QAbstractSocket::ConnectedState))
        m_pRetryTimer->start(1000);
}
