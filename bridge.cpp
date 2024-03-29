#include <QFile>
#include <QDebug>
#include <QCoreApplication>
#include <QStringList>
#include <QString>
#include <QStateMachine>
#include <QState>
#include <QFinalState>
#include <QSPIDevice>
#include <math.h>


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
    m_bOscilloscopeCmd = false;
    m_bParameterCmd = false;
    m_bfirstParameterSend = false;

    m_nRecoveryCount = 0;
    m_nRecoveryCountTotal = 0;

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

    syncTimer.setSingleShot(true);
    syncTimer.setInterval(10);

    rangeRecoveryTimer.setSingleShot(true);
    rangeRecoveryTimer.setInterval(5000); // we will retry for 5 seconds after fg301 command to set correct ranges
    connect(&rangeRecoveryTimer, SIGNAL(timeout()), SLOT(rangeRecoveryExpired()));

    m_pBridgeConfigStateMachine->start();
    qInfo("com5003-lwl-bridge started");
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
        delete m_pBridgeActiveParameterStartState;
        delete m_pBridgeActiveParameterDoneState;
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

    m_pSocket = m_pETHConnection->getSocket();

    parameterDelegate = new cETHParameterDelegate(m_pSocket);
    measureDelegate = new cETHMeasureDelegate(m_pSocket);
    oscilloscopeDelegate = new cETHOscilloscopeDelegate(m_pSocket, measureDelegate);

    m_pBridgeStateMachine = new QStateMachine();
    m_pBridgeIdleState = new QState();
    m_pBridgeLWLConnectedState = new QState();
    m_pBridgeETHConnectedState = new QState();
    m_pBridgeActiveState = new QState();

    m_pBridgeActiveInitState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveInitDoneState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveMeasureStartState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveMeasureDoneState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveParameterStartState = new QState(m_pBridgeActiveState);
    m_pBridgeActiveParameterDoneState = new QState(m_pBridgeActiveState);
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
    m_pBridgeActiveMeasureDoneState->addTransition(this, SIGNAL(startParameter()), m_pBridgeActiveParameterStartState);
    m_pBridgeActiveParameterStartState->addTransition(parameterDelegate, SIGNAL(finished()), m_pBridgeActiveParameterDoneState);
    m_pBridgeActiveParameterDoneState->addTransition(this, SIGNAL(startMeasurement()), m_pBridgeActiveMeasureStartState);
    m_pBridgeActiveParameterDoneState->addTransition(this, SIGNAL(startOscilloscope()), m_pBridgeActiveOscilloscopeStartState);
    m_pBridgeActiveOscilloscopeStartState->addTransition(oscilloscopeDelegate, SIGNAL(finished()), m_pBridgeActiveOscilloscopeDoneState);
    m_pBridgeActiveOscilloscopeDoneState->addTransition(this, SIGNAL(syncFG301()), m_pBridgeActiveOscilloscopeSyncState);
    m_pBridgeActiveOscilloscopeSyncState->addTransition(&syncTimer, SIGNAL(timeout()), m_pBridgeActiveOscilloscopeSyncState);
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
    connect(m_pBridgeActiveParameterStartState, SIGNAL(entered()), SLOT(bridgeActiveParameterStart()));
    connect(m_pBridgeActiveParameterDoneState, SIGNAL(entered()), SLOT(bridgeActiveParameterDone()));
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
    m_bParameterCmd = false;
}


void cBridge::bridgeIdle()
{
#ifdef DEBUGInit
    qDebug() << "Bridge idle state entered";
#endif
    m_pSPIConnection->setStatus(bridgeIdleStatus);
}


void cBridge::bridgeLWLConnected()
{
#ifdef DEBUGInit
    qDebug() << "Bridge LWL connected state entered";
#endif
    m_pSPIConnection->setStatus(bridgeLWLConnectedStatus);
}


void cBridge::bridgeETHConnected()
{
#ifdef DEBUGInit
    qDebug() << "Bridge ETH connected state entered";
#endif
    m_pSPIConnection->setStatus(bridgeETHConnectedStatus);
}


void cBridge::bridgeActiveInit()
{    
#ifdef DEBUGInit
    qDebug() << "Bridge active init state entered";
#endif

    m_pSPIConnection->setStatus(bridgeActiveStatus);

    // we have to set some default values , info about these comes from xml config file

    m_bActive = true;

    QList<QString> cmdList;

    cmdList.append(QString("conf:rng1:pll %1;\n").arg(m_pBridgeConfigData->m_bPLLAuto));
    cmdList.append(QString("conf:rng1:rng %1;\n").arg(m_pBridgeConfigData->m_bRangeAuto));
    cmdList.append(QString("conf:rng1:gro %1;\n").arg(m_pBridgeConfigData->m_bGrouping));
    cmdList.append(QString("conf:rms1:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:dft1:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow1:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow2:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow3:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));
    cmdList.append(QString("conf:pow4:tint %1;\n").arg(m_pBridgeConfigData->m_nIntegrationtime));

    parameterDelegate->setCmdList(cmdList);
    parameterDelegate->execute();
}


void cBridge::bridgeActiveInitDone()
{
#ifdef DEBUGInit
    qDebug() << "Bridge active done state entered";
#endif
    // we got lwlconnected and eth connected so
    setParameterCommands(); // once we call from here, later we get a signal each time lwl data has changed
    m_fUBValueSet = m_fUBValueWanted;
    m_fIBValueSet = m_fIBValueWanted;

    parameterDelegate->execute();
}


void cBridge::bridgeLWLCommand()
{
    // data has been read from lwlconnection and we have to
    // derive some commands from data and send them to the reference meter
#ifdef DEBUGCmd
    qDebug() << QTime::currentTime() << ": " << "Bridge fg301 lwl command received";
#endif

    setParameterCommands();
    m_bfirstParameterSend = true;
    m_bParameterCmd = true;
    m_nRecoveryCount = 0; // after each command we reset the recovery count

#ifdef DEBUGRange
                qDebug() << QTime::currentTime() << ": " << QString("RecoveryCountTotal:%1").arg(m_nRecoveryCountTotal);
#endif

}


void cBridge::setParameterCommands()
{
    QByteArray lwlInput;
    int selCode;
    QList<QString> cmdList;
    double scale;

    lwlInput = m_pLWLConnection->getLWLInput();

    // we make some tests here on parameter codes, in case they are corrupted we don't want to crash

    QString s;
    selCode = lwlInput[UBCode];
    if (!m_pBridgeConfigData->m_VoltageRangeHash.contains(selCode))
        selCode = 0;

    s = m_pBridgeConfigData->m_VoltageRangeHash[selCode];
    s.replace("V","");
    if (s.contains('m') > 0)
    {
        s.replace("m","");
        scale = 0.001;
    }
    else
        scale = 1.0;

    m_fUBValueWanted = s.toDouble() * scale;

    cmdList.append(s = QString("sens:rng1:ul1:rang %1;\n").arg(m_pBridgeConfigData->m_VoltageRangeHash[selCode]));
#ifdef DEBUGRange
    qDebug() << QTime::currentTime() << ": " << QString("Cmd :%1").arg(s);
#endif

    selCode = lwlInput[IBCode];
    if (!m_pBridgeConfigData->m_CurrentRangeHash.contains(selCode))
        selCode = 0;

    s = m_pBridgeConfigData->m_CurrentRangeHash[selCode];
    s.replace("A","");
    if (s.contains('m') > 0)
    {
        s.replace("m","");
        scale = 0.001;
    }
    else
        scale = 1.0;

    m_fIBValueWanted = s.toDouble() * scale;

    cmdList.append(s = QString("sens:rng1:il1:rang %1;\n").arg(m_pBridgeConfigData->m_CurrentRangeHash[selCode]));
#ifdef DEBUGRange
    qDebug() << QTime::currentTime() << ": " << QString("Cmd :%1").arg(s);
#endif

    // we set all configuration listed measuring modes related to MMCode
    cModeSelect mSelect;
    selCode = lwlInput[MMCode];
    if (!m_pBridgeConfigData->m_MeasuringmodeHash.contains(selCode))
        selCode = 0;
    mSelect = m_pBridgeConfigData->m_MeasuringmodeHash[selCode];
    for (int i = 0; i < mSelect.m_sModuleNameList.count(); i++)
        cmdList.append(QString("conf:%1:mmod %2;\n").arg(mSelect.m_sModuleNameList.at(i)).arg(mSelect.m_sMeasmodeNameList.at(i)));

    measureDelegate->setAngleReference(lwlInput[AngleRefCode]);

    parameterDelegate->setCmdList(cmdList);

    int osciChannel = lwlInput[OsciCmd] & 0x7f;

    if (!m_bOscilloscopeCmd) // retrigger impossible
    {
        if (osciChannel > 0) // we have to start the oscilloscope now (statemachine)
        {
            oscilloscopeDelegate->setChannel(osciChannel);
            m_bOscilloscopeCmd = true;
        }
    }
}


void cBridge::bridgeActiveMeasureStart()
{
#ifdef DEBUGMeas
    qDebug() << "Bridge measure start state entered";
#endif
    measureDelegate->execute();
}


void cBridge::bridgeActiveMeasureDone()
{
#ifdef DEBUGMeas
    qDebug() << "Bridge measure done state entered";
#endif
    QHash<QString, double*> ActValueHash;

    ActValueHash = measureDelegate->getActualValues();
    m_pLWLConnection->sendActualValues(ActValueHash);

#ifdef DEBUGRange
    // while recovery timer is running we want to see the range info and the according time elapsed since range command
    if (rangeRecoveryTimer.isActive())
    {
        int t;
        t = rangeTime.elapsed();

        qDebug() << QTime::currentTime() << ": " << QString("VoltageRangeInfo %1 dt[ms]=%2").arg(*(ActValueHash["UB"]), 6, 'f', 2).arg(t);
        if ( (*(ActValueHash["IB"]) < 1.0))
            qDebug() << QTime::currentTime() << ": " << QString("CurrentRangeInfo %1 dt[ms]=%2").arg(*(ActValueHash["IB"]), 6, 'f', 3).arg(t);
        else
            qDebug() << QTime::currentTime() << ": " << QString("CurrentRangeInfo %1 dt[ms]=%2").arg(*(ActValueHash["IB"]), 6, 'f', 2).arg(t);
    }
#endif

    if ( !(fabs((*ActValueHash["UB"]) - m_fUBValueSet) < 1e-7) || !(fabs((*ActValueHash["IB"]) - m_fIBValueSet) < 1e-7) )
    // a voltage or current range is not what we wanted
        if (rangeRecoveryTimer.isActive())
        {
            if (*ActValueHash["OVL"] > 0.0) // reference meter has overload
            {
                setParameterCommands();
                m_bParameterCmd = true;
                m_nRecoveryCount++;
                m_nRecoveryCountTotal++;
#ifdef DEBUGRange
                qDebug() << QTime::currentTime() << ": " << QString("RecoveryCount:%1").arg(m_nRecoveryCount);
#endif
            }
            else
            {
#ifdef DEBUGRange
                qDebug() << QTime::currentTime() << ": " << QString("Wrong range no overload condition");
#endif
            }
        }
        else
        {
#ifdef DEBUGRange
            qDebug() << QTime::currentTime() << ": " << QString("Wrong range recoverytimer expired") << QString("RangeRead: %1").arg(*ActValueHash["IB"]) << QString("RangeWanted: %1").arg(m_fIBValueSet);
#endif
        }

    if (m_bParameterCmd)
    {
        m_bParameterCmd = false;
        rangeTime.start();
        emit startParameter();
    }

    else
       emit startMeasurement();
}


void cBridge::bridgeActiveParameterStart()
{
#ifdef DEBUGPar
    qDebug() << QTime::currentTime() << ": " << "Bridge parameter start state entered";
#endif
    parameterDelegate->execute();
}


void cBridge::bridgeActiveParameterDone()
{
#ifdef DEBUGPar
    qDebug() << QTime::currentTime() << ": " << "Bridge parameter done state entered";
#endif
    m_fUBValueSet = m_fUBValueWanted;
    m_fIBValueSet = m_fIBValueWanted;

    if (m_bfirstParameterSend)
    {
        m_bfirstParameterSend = false; // it was the first send of parameter after new lwl config.
        rangeRecoveryTimer.start(); // after we sent parameters we start our recovery timer
    }

    if (m_bOscilloscopeCmd)
        emit startOscilloscope();
    else
        emit startMeasurement();
}


void cBridge::bridgeActiveOscilloscopeStart()
{
#ifdef DEBUGMeas
    qDebug() << "Bridge oscilloscope start state entered";
#endif
    m_pLWLConnection->sendCmdRecognized(true); // we tell fg301 that we have recognized its command
    oscilloscopeDelegate->execute(); // and do
}


void cBridge::bridgeActiveOscilloscopeDone()
{
#ifdef DEBUGMeas
    qDebug() << "Bridge oscilloscope done state entered";
#endif
    m_pLWLConnection->sendOscillogram(oscilloscopeDelegate->getOscillogram());
    emit syncFG301();
}


void cBridge::bridgeActiveOscilloscopeSync()
{
#ifdef DEBUGMeas
    qDebug() << "Bridge oscilloscope sync state entered";
#endif

    QByteArray &lwlinput = m_pLWLConnection->getLWLInput();

    if (lwlinput.at(OsciCmd) > 0)
    {
        syncTimer.start();
    }
    else
    {
        m_pLWLConnection->sendCmdRecognized(false);
        m_bOscilloscopeCmd = false;
        emit startMeasurement();
    }
}

void cBridge::bridgeError(int errNum)
{
    QCoreApplication::instance()->exit(errNum); // we leave application on error condition
}


void cBridge::rangeRecoveryExpired()
{
    rangeRecoveryTimer.stop();
}
