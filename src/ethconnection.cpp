#include <QTcpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QString>
#include <QDebug>

#include "ethconnection.h"
#include "bridgeconfigdata.h"

cETHConnection::cETHConnection(cBridgeConfigData *configdata)
    :m_pConfigData(configdata)
{
    m_pConnectTimer = new QTimer();
    m_pConnectTimer->setSingleShot(false);
    connect(m_pConnectTimer, SIGNAL(timeout()), this, SLOT(try2Connect()));
    m_pSocket = new QTcpSocket(this);
    connect(m_pSocket, SIGNAL(connected()), this, SLOT(regConnection()));
    connect(m_pSocket, SIGNAL(disconnected()), this, SLOT(regDisconnection()));
    connect(m_pSocket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(regError(QAbstractSocket::SocketError)));
    m_pConnectTimer->start(500);
}


cETHConnection::~cETHConnection()
{
    delete m_pConnectTimer;
    delete m_pSocket;
}


QTcpSocket *cETHConnection::getSocket()
{
    return m_pSocket;
}


void cETHConnection::try2Connect()
{
    m_pSocket->connectToHost(QHostAddress(m_pConfigData->m_sIPReferenceMeter), m_pConfigData->m_nPortReferenceMeter);
}


void cETHConnection::regConnection()
{
    m_pConnectTimer->stop();
    emit connected();
}


void cETHConnection::regDisconnection()
{
    m_pConnectTimer->start(500);
    emit disconnected();
}

void cETHConnection::regError(QAbstractSocket::SocketError err)
{
    qDebug() << QString("Bridge ethernet connection error %1").arg(err);
}
