#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QSPIDevice>

#include "bridge.h"
#include "lwlconnection.h"
#include "ethconnection.h"
#include "bridgeconfiguration.h"
#include "bridgeconfigdata.h"
#include "ethparameterdelegate.h"
#include "ethmeasuredelegate.h"
#include "ethoscilloscopedelegate.h"
#include "ethcmdserializer.h"
#include "spiconnection.h"



cBridge::cBridge()
{
    m_bActive = false;
    m_pBridgeConfiguration = new cBridgeConfiguration();

    m_pBridgeConfigStateMachine = new QStateMachine();
    m_pBridgeConfigurationState = new QState();
    m_pBridgeConfigureDoneState = new QFinalState();

    m_pBridgeConfigurationState->addTransition(m_pBridgeConfiguration, SIGNAL(configXMLDone()), m_pBridgeConfigureDoneState);

    m_pBridgeConfigStateMachine->addState(m_pBridgeConfigurationState);
    m_pBridgeConfigStateMachine->addState(m_pBridgeConfigureDoneState);

    m_pBridgeConfigStateMachine->setInitialState(m_pBridgeConfigurationState);

    connect(m_pBridgeConfigurationState, SIGNAL(entered()), SLOT(bridgeConfiguration()));
    connect(m_pBridgeConfigureDoneState, SIGNAL(entered()), SLOT(bridgeConfigurationDone()));
    connect(m_pBridgeConfiguration, SIGNAL(error(int)), this, SLOT(bridgeError(int)));

    m_pBridgeConfigStateMachine->start();
}


cBridge::~cBridge()
{
    delete m_pBridgeConfiguration;
    delete m_pBridgeConfigurationState;
    delete m_pBridgeConfigureDoneState;
    delete m_pBridgeConfigStateMachine;

    if (m_pLWLConnection)
        delete m_pLWLConnection;
    if (m_pETHConnection)
        delete m_pETHConnection;

    if (m_pSPICtrlDevice)
    {
        m_pSPICtrlDevice->close();
        delete m_pSPICtrlDevice;
    }

    if (m_pSPIDataDevice)
    {
        m_pSPIDataDevice->close();
        delete m_pSPIDataDevice;
    }

    if (m_pSPIConnection)
        delete m_pSPIConnection;

    if (m_pBridgeStateMachine)
    {
        delete m_pBridgeActiveMeasureStartState;
        delete m_pBridgeActiveMeasureDoneState;
        delete m_pBridgeActiveOscilloscopeStartState;
        delete m_pBridgeActiveOscilloscopeDoneState;
        delete m_pBridgeActiveOscilloscopeSyncState;
        delete m_pBridgeActiveInitState;
        delete m_pBridgeActiveState;
        delete m_pBridgeETHConnectedState;
        delete m_pBridgeLWLConnectedState;
        delete m_pBridgeIdleState;
        delete m_pBridgeStateMachine;

        delete parameterDelegate;
        delete measureDelegate;
        delete oscilloscopeDelegate;
    }


}


void cBridge::bridgeConfiguration()
{
    QFile *file;
    QByteArray xmlConfigData;

    file = new QFile(defaultXMLFile);
    if (file->open(QIODevice::Unbuffered | QIODevice::ReadOnly))
    {
        xmlConfigData = file->readAll();
        file->close();
        m_pBridgeConfiguration->setConfiguration(xmlConfigData);
    }

    delete file;
}


void cBridge::bridgeConfigurationDone()
{
    // if configuration is done we setup our connections and the bridges real statemachine

    m_pBridgeConfigData = m_pBridgeConfiguration->getConfigurationData();

    m_pSPICtrlDevice = new QSPIDevice(m_pBridgeConfigData->m_sSPICtrlDeviceName);
    if (!m_pSPICtrlDevice->open(QIODevice::ReadWrite))
    {
        bridgeError(-1); // we cancel program if we don't find specified spi device
        return;
    }

    if (!m_pSPICtrlDevice->setBitSpeed(16000000)) // see BB-SPIDEVx-00A0.dts
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPICtrlDevice->setMode(3))
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPICtrlDevice->setLSBFirst(false))
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPICtrlDevice->setBitsPerWord(8))
    {
        bridgeError(-1);
        return;
    }

    m_pSPIDataDevice = new QSPIDevice(m_pBridgeConfigData->m_sSPIDataDeviceName);
    if (!m_pSPIDataDevice->open(QIODevice::ReadWrite))
    {
        bridgeError(-1); // we cancel program if we don't find specified spi device
        return;
    }

    if (!m_pSPIDataDevice->setBitSpeed(16000000)) // see BB-SPIDEVx-00A0.dts
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPIDataDevice->setMode(3))
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPIDataDevice->setLSBFirst(false))
    {
        bridgeError(-1);
        return;
    }

    if(!m_pSPIDataDevice->setBitsPerWord(8))
    {
        bridgeError(-1);
        return;
    }

    m_pSPIConnection = new cSPIConnection(m_pSPICtrlDevice, m_pSPIDataDevice);
    m_pLWLConnection = new cLWLConnection(m_pSPIConnection);
    m_pETHConnection = new cETHConnection(m_pBridgeConfigData);
    m_pCmdSerializer = new cETHCmdSerializer();

    m_pSocket = m_pETHConnection->getSocket();

    parameterDelegate = new cETHParameterDelegate();
    measureDelegate = new cETHMeasureDelegate();
    oscilloscopeDelegate = new cETHOscilloscopeDelegate;

    m_pBridgeStateMachine = new QStateMachine();
    m_pBridgeIdleState = new QState();
    m_pBridgeLWLConnectedState = new QState();
    m_pBridgeETHConnectedState = new QState();
    m_pBridgeActiveState = new QState();

    m_pBridgeActiveInitState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveInitDoneState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveMeasureStartState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveMeasureDoneState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveOscilloscopeStartState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveOscilloscopeDoneState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveOscilloscopeSyncState = new QState(m_pBridgeActiveState);

    m_pBridgeActiveState->setInitialState(m_pBridgeActiveInitState);

    m_pBridgeIdleState->addTransition(m_pLWLConnection, SIGNAL(connected()), m_pBridgeLWLConnectedState);
    m_pBridgeLWLConnectedState->addTransition(m_pLWLConnection, SIGNAL(disconnected()), m_pBridgeIdleState);
    m_pBridgeLWLConnectedState->addTransition(m_pETHConnection, SIGNAL(connected()), m_pBridgeActiveState);
    m_pBridgeIdleState->addTransition(m_pETHConnection, SIGNAL(connected()), m_pBridgeETHConnectedState);
    m_pBridgeETHConnectedState->addTransition(m_pETHConnection, SIGNAL(disconnected()), m_pBridgeIdleState);
    m_pBridgeETHConnectedState->addTransition(m_pLWLConnection, SIGNAL(connected()), m_pBridgeActiveState);
    m_pBridgeActiveState->addTransition(m_pLWLConnection, SIGNAL(disconnected()), m_pBridgeETHConnectedState);
    m_pBridgeActiveState->addTransition(m_pETHConnection, SIGNAL(disconnected()), m_pBridgeLWLConnectedState);
    m_pBridgeActiveInitState->addTransition(parameterDelegate, SIGNAL(finished()), m_pBridgeActiveInitDoneState);
    m_pBridgeActiveInitDoneState->addTransition(parameterDelegate, SIGNAL(finished()), m_pBridgeActiveMeasureStartState);
    m_pBridgeActiveMeasureStartState->addTransition(measureDelegate, SIGNAL(finished()), m_pBridgeActiveMeasureDoneState);
    m_pBridgeActiveMeasureDoneState->addTransition(this, SIGNAL(startMeasurement()), m_pBridgeActiveMeasureStartState);
    m_pBridgeActiveMeasureDoneState->addTransition(this, SIGNAL(startOscilloscope()), m_pBridgeActiveOscilloscopeStartState);
    m_pBridgeActiveOscilloscopeStartState->addTransition(oscilloscopeDelegate, SIGNAL(finished()), m_pBridgeActiveOscilloscopeDoneState);
    m_pBridgeActiveOscilloscopeDoneState->addTransition(this, SIGNAL(syncFG301()), m_pBridgeActiveOscilloscopeSyncState);
    m_pBridgeActiveOscilloscopeSyncState->addTransition(this, SIGNAL(syncFG301()), m_pBridgeActiveOscilloscopeSyncState);
    m_pBridgeActiveOscilloscopeSyncState->addTransition(this, SIGNAL(startMeasurement()), m_pBridgeActiveMeasureStartState);

    m_pBridgeStateMachine->addState(m_pBridgeIdleState);
    m_pBridgeStateMachine->addState(m_pBridgeLWLConnectedState);
    m_pBridgeStateMachine->addState(m_pBridgeETHConnectedState);
    m_pBridgeStateMachine->addState(m_pBridgeActiveState);

    m_pBridgeStateMachine->setInitialState(m_pBridgeIdleState);

    connect(m_pBridgeIdleState, SIGNAL(entered()), SLOT(bridgeIdle()));
    connect(m_pBridgeActiveState, SIGNAL(exited()), SLOT(bridgeInactive()));
    connect(m_pBridgeActiveInitState, SIGNAL(entered()), SLOT(bridgeActiveInit()));
    connect(m_pBridgeActiveInitDoneState, SIGNAL(entered()), SLOT(bridgeActiveInitDone()));
    connect(m_pBridgeLWLConnectedState, SIGNAL(entered()), SLOT(bridgeLWLConnected()));
    connect(m_pBridgeETHConnectedState, SIGNAL(entered()), SLOT(bridgeETHConnected()));
    connect(m_pBridgeActiveMeasureStartState, SIGNAL(entered()), SLOT(bridgeActiveMeasureStart()));
    connect(m_pBridgeActiveMeasureDoneState, SIGNAL(entered()), SLOT(bridgeActiveMeasureDone()));
    connect(m_pBridgeActiveOscilloscopeStartState, SIGNAL(entered()), SLOT(bridgeActiveOscilloscopeStart()));
    connect(m_pBridgeActiveOscilloscopeDoneState, SIGNAL(entered()), SLOT(bridgeActiveOscilloscopeDone()));
    connect(m_pBridgeActiveOscilloscopeSyncState, SIGNAL(entered()), SLOT(bridgeActiveOscilloscopeSync()));

    connect(m_pLWLConnection, SIGNAL(error(int)), this, SLOT(bridgeError(int)));
    connect(m_pLWLConnection,SIGNAL(dataAvail()), this, SLOT(bridgeLWLCommand()));
    connect(m_pETHConnection, SIGNAL(error(int)), this, SLOT(bridgeError(int)));

    m_pBridgeStateMachine->start();
}


void cBridge::bridgeInactive()
{
    m_bActive = false;
    m_bOscilloscopeCmd = false;
    m_pBridgeStateMachine->stop();
}


void cBridge::bridgeIdle()
{
#ifdef DEBUG
    qDebug() << "Bridge idle state entered";
#endif
}


void cBridge::bridgeLWLConnected()
{
#ifdef DEBUG
    qDebug() << "Bridge LWL connected state entered";
#endif
}


void cBridge::bridgeETHConnected()
{
#ifdef DEBUG
    qDebug() << "Bridge ETH connected state entered";
#endif
}


void cBridge::bridgeActiveInit()
{    
#ifdef DEBUG
    qDebug() << "Bridge active state entered";
#endif

    // we have to set some default values , info about these comes from xml config file

    m_bActive = true;
    m_bOscilloscopeCmd = false;

    QList<QString> cmdList;

    cmdList.append(QString("conf:rng1:rng %1;\n").arg(m_pBridgeConfigData->m_bRangeAuto));
    cmdList.append(QString("conf:rng1:gro %1;\n").arg(m_pBridgeConfigData->m_bGrouping));
    cmdList.append(QString("conf:rms1:tint %1;n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:dft1:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow1:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow2:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow3:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow4:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));

    parameterDelegate->setCmdList(cmdList);

    // each time bridge becomes active we must set the socket for all delegates
    parameterDelegate->setSocket(m_pSocket);
    measureDelegate->setSocket(m_pSocket);
    oscilloscopeDelegate->setSocket(m_pSocket);

    m_pCmdSerializer->execute(parameterDelegate);
}


void cBridge::bridgeActiveInitDone()
{
    // we got lwlconnected and eth connected so
    bridgeLWLCommand(); // once we call from here, later we get a signal each time lwl data has changed
}


void cBridge::bridgeLWLCommand()
{
    // we have to read data from lwlconnection
    // derive some commands from data and send them to the reference meter

    QByteArray lwlInput;

    lwlInput = m_pLWLConnection->getLWLInput();
    QList<QString> cmdList;

    cmdList.append(QString("sens:rng1:ul1:rang %1;\n").arg(m_pBridgeConfigData->m_VoltageRangeHash[lwlInput[UBCode]]));
    cmdList.append(QString("sens:rng1:il1:rang %1;\n").arg(m_pBridgeConfigData->m_CurrentRangeHash[lwlInput[IBCode]]));

    // we set all configuration listed measuring modes related to MMCode
    cModeSelect mSelect;
    mSelect = m_pBridgeConfigData->m_MeasuringmodeHash[lwlInput[MMCode]];
    for (int i = 0; i < mSelect.m_sModuleNameList.count(); i++)
        cmdList.append(QString("conf:%1:mmod %2;\n").arg(mSelect.m_sModuleNameList.at(i)).arg(mSelect.m_sMeasmodeNameList.at(i)));

    measureDelegate->setAngleReference(lwlInput[AngleRefCode]);

    parameterDelegate->setCmdList(cmdList);
    m_pCmdSerializer->execute(parameterDelegate);

    int osciChannel = lwlInput[OsciCmd];

    if (osciChannel > 0) // we have to start the oscilloscope now (statemachine)
    {
        oscilloscopeDelegate->setChannel(osciChannel);
        m_bOscilloscopeCmd = true;
    }
}


void cBridge::bridgeActiveMeasureStart()
{
    m_pCmdSerializer->execute(measureDelegate);
}


void cBridge::bridgeActiveMeasureDone()
{
    m_pLWLConnection->sendActualValues(measureDelegate->getActualValues());
    if (m_bOscilloscopeCmd)
    {
        m_bOscilloscopeCmd = false;
        emit startOscilloscope();
    }
    else
        emit startMeasurement();
}


void cBridge::bridgeActiveOscilloscopeStart()
{
    m_pLWLConnection->sendCmdRecognized(true); // we tell fg301 that we have recognized its command
    oscilloscopeDelegate->execute(); // and do
}


void cBridge::bridgeActiveOscilloscopeDone()
{
    m_pLWLConnection->sendOscillogram(oscilloscopeDelegate->getOscillogram());
    emit syncFG301();
}


void cBridge::bridgeActiveOscilloscopeSync()
{
    QByteArray &lwlinput = m_pLWLConnection->getLWLInput();

    if (lwlinput.at(OsciCmd) > 0)
        emit syncFG301();
    else
    {
        m_pLWLConnection->sendCmdRecognized(false);
        emit startMeasurement();
    }
}


void cBridge::bridgeError(int errNum)
{
    QCoreApplication::instance()->exit(errNum); // we leave application on error condition
}
