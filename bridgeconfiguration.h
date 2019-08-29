#ifndef BRIDGECONFIGURATION
#define BRIDGECONFIGURATION

#include <QObject>
#include <QMap>

namespace  Zera
{
namespace XMLConfig
{
    class cReader;
}
}

enum moduleconfigstate
{
    setDebugLevel,
    setReferenceMeterIp,
    setReferenceMeterPort,
    setSPICtrlDeviceName,
    setSPIDataDeviceName,
    setPllAuto,
    setRangeAuto,
    setGrouping,
    setIntegrationtime,
    setVoltagerangeCount,
    setCurrentrangeCount,
    setMeasuringmodeCount,

    setVoltagerange1 = 16, // we leave place for up to 32 ranges .....
    setCurrentrange1 = 48,
    setMeasuringmode1 = 80
};


class cBridgeConfigData;

const QString defaultXSDFile = "/etc/zera/com5003lwlbridge/bridge.xsd";
const QString defaultXMLFile = "/etc/zera/com5003lwlbridge/bridge.xml";


class cBridgeConfiguration: public QObject
{
    Q_OBJECT
public:
    cBridgeConfiguration();
    ~cBridgeConfiguration();
    virtual void setConfiguration(QByteArray xmlString);
    bool isConfigured();
    cBridgeConfigData* getConfigurationData();

signals:
    void configXMLDone();
    void error(int);

protected slots:
    virtual void configXMLInfo(QString key);
    virtual void completeConfiguration(bool ok);

private:
    QMap<QString,quint16> m_ConfigXMLMap;
    Zera::XMLConfig::cReader* m_pXMLReader;
    cBridgeConfigData *m_pBridgeConfigData;  // configuration
    bool m_bConfigured;
    bool m_bConfigError;
};




#endif // BRIDGECONFIGURATION

