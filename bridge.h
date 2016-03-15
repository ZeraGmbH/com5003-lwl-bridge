#ifndef BRIDGE
#define BRIDGE

#include <QObject>
#include <QHash>
#include <QStringList>
#include <QTimer>


class QStateMachine;
class QState;
class QFinalState;
class QTcpSocket;

class cLWLConnection;
class cETHConnection;
class cBridgeConfiguration;
class cBridgeConfigData;
class cETHParameterDelegate;
class cETHMeasureDelegate;
class cETHOscilloscopeDelegate;

#define DEBUG 1

enum bridgeExitConditions
{
    configError = 1
};

enum lwlDataPositions
{
    UBCode = 0,
    IBCode = 1,
    MMCode = 2,
    AngleRefCode = 3,
    OsciCmd = 28
};

class cSPIConnection;
class QSPIDevice;

class cBridge: public QObject
{
    Q_OBJECT

public:
    cBridge();
    ~cBridge();

signals:
    void startMeasurement();
    void startParameter();
    void startOscilloscope();
    void syncFG301();
    void syncFG301Loop();
    void error(int);

private:
    bool m_bActive;
    bool m_bOscilloscopeCmd;
    bool m_bParameterCmd;

    cBridgeConfiguration *m_pBridgeConfiguration;
    cBridgeConfigData *m_pBridgeConfigData;

    cSPIConnection *m_pSPIConnection;
    cLWLConnection *m_pLWLConnection;
    cETHConnection *m_pETHConnection;
    QTcpSocket *m_pSocket;
    QSPIDevice *m_pSPICtrlDevice;
    QSPIDevice *m_pSPIDataDevice;
    cETHParameterDelegate *parameterDelegate;
    cETHMeasureDelegate *measureDelegate;
    cETHOscilloscopeDelegate *oscilloscopeDelegate;


    // state machine for bridge configuration
    QStateMachine *m_pBridgeConfigStateMachine;
    QState *m_pBridgeConfigurationState;
    QFinalState *m_pBridgeConfigureDoneState;

    // state machine for setup, control and execution
    QStateMachine *m_pBridgeStateMachine;
    QState *m_pBridgeIdleState;
    QState *m_pBridgeLWLConnectedState;
    QState *m_pBridgeETHConnectedState;
    QState *m_pBridgeActiveState;

    QState *m_pBridgeActiveInitState;
    QState *m_pBridgeActiveInitDoneState;
    QState *m_pBridgeActiveMeasureStartState;
    QState *m_pBridgeActiveMeasureDoneState;
    QState *m_pBridgeActiveParameterStartState;
    QState *m_pBridgeActiveParameterDoneState;
    QState *m_pBridgeActiveOscilloscopeStartState;
    QState *m_pBridgeActiveOscilloscopeDoneState;
    QState *m_pBridgeActiveOscilloscopeSyncState;

    QTimer syncTimer;

private slots:
    void bridgeConfiguration();
    void bridgeConfigurationDone();
    void bridgeInactive();

    void bridgeIdle();
    void bridgeLWLConnected();
    void bridgeETHConnected();
    void bridgeActiveInit();
    void bridgeActiveInitDone();
    void bridgeActiveMeasureStart();
    void bridgeActiveMeasureDone();
    void bridgeActiveParameterStart();
    void bridgeActiveParameterDone();
    void bridgeActiveOscilloscopeStart();
    void bridgeActiveOscilloscopeDone();
    void bridgeActiveOscilloscopeSync();

    void bridgeLWLCommand();
    void setParameterCommands();




    void bridgeError(int errNum);

};

#endif // BRIDGE

