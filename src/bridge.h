#ifndef BRIDGE
#define BRIDGE

#include <QObject>
#include <QHash>
#include <QStringList>


class QStateMachine;
class QState;
class QFinalState;
class QTcpSocket;

class cLWLConnection;
class cETHConnection;
class cBridgeConfiguration;
class cBridgeConfigData;
class cETHCmdSerializer;
class cETHParameterDelegate;
class cETHMeasureDelegate;
class cETHOscilloscopeDelegate;

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
    OsciCmd = 29
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
    void startOscilloscope();
    void syncFG301();
    void error(int);

private:
    bool m_bActive;
    bool m_bOscilloscopeCmd;
    cBridgeConfiguration *m_pBridgeConfiguration;
    cBridgeConfigData *m_pBridgeConfigData;

    cSPIConnection *m_pSPIConnection;
    cLWLConnection *m_pLWLConnection;
    cETHConnection *m_pETHConnection;
    QTcpSocket *m_pSocket;
    QSPIDevice *m_pSPICtrlDevice;
    QSPIDevice *m_pSPIDataDevice;
    cETHCmdSerializer *m_pCmdSerializer;
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
    QState *m_pBridgeActiveMeasureStartState;
    QState *m_pBridgeActiveMeasureDoneState;
    QState *m_pBridgeActiveOscilloscopeStartState;
    QState *m_pBridgeActiveOscilloscopeDoneState;
    QState *m_pBridgeActiveOscilloscopeSyncState;

private slots:
    void bridgeConfiguration();
    void bridgeConfigurationDone();
    void bridgeInactive();

    void bridgeActiveInit();
    void bridgeActiveMeasureStart();
    void bridgeActiveMeasureDone();
    void bridgeActiveOscilloscopeStart();
    void bridgeActiveOscilloscopeDone();
    void bridgeActiveOscilloscopeSync();

    void bridgeLWLCommand();




    void bridgeError(int errNum);

};

#endif // BRIDGE

