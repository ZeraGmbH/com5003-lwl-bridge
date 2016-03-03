#include <QStringList>

#include <xmlconfigreader.h>

#include "bridge.h"
#include "bridgeconfiguration.h"
#include "bridgeconfigdata.h"


cBridgeConfiguration::cBridgeConfiguration()
{
    m_pBridgeConfigData = 0;
    m_pXMLReader = new Zera::XMLConfig::cReader(this);
    connect(m_pXMLReader, SIGNAL(valueChanged(const QString&)), this, SLOT(configXMLInfo(const QString&)));
    connect(m_pXMLReader, SIGNAL(finishedParsingXML(bool)), this, SLOT(completeConfiguration(bool)));
}


cBridgeConfiguration::~cBridgeConfiguration()
{
    if (m_pBridgeConfigData) delete m_pBridgeConfigData;
}


void cBridgeConfiguration::setConfiguration(QByteArray xmlString)
{
    m_bConfigured = m_bConfigError = false;

    if (m_pBridgeConfigData) delete m_pBridgeConfigData;
    m_pBridgeConfigData = new cBridgeConfigData();

    m_ConfigXMLMap.clear(); // in case of new configuration we completely set up

    // so now we can set up
    // initializing hash table for xml configuration

    m_ConfigXMLMap["bridgeconf:debuglevel"] = setDebugLevel;
    m_ConfigXMLMap["bridgeconf:referencemeter:ip"] = setReferenceMeterIp;
    m_ConfigXMLMap["bridgeconf:referencemeter:port"] = setReferenceMeterPort;
    m_ConfigXMLMap["bridgeconf:referencemeter:spictrl"] = setSPICtrlDeviceName;
    m_ConfigXMLMap["bridgeconf:referencemeter:spidata"] = setSPIDataDeviceName;
    m_ConfigXMLMap["bridgeconf:referencemeter:rangeauto"] = setRangeAuto;
    m_ConfigXMLMap["bridgeconf:referencemeter:grouping"] = setGrouping;
    m_ConfigXMLMap["bridgeconf:referencemeter:integrationtime"] = setIntegrationtime;
    m_ConfigXMLMap["bridgeconf:referencemeter:voltagerange:n"] = setVoltagerangeCount;
    m_ConfigXMLMap["bridgeconf:referencemeter:currentrange:n"] = setCurrentrangeCount;
    m_ConfigXMLMap["bridgeconf:referencemeter:measuringmode:n"] = setMeasuringmodeCount;

    if (m_pXMLReader->loadSchema(defaultXSDFile))
        m_pXMLReader->loadXMLFromString(QString::fromUtf8(xmlString.data(), xmlString.size()));
    else
        emit error(configError);
}


bool cBridgeConfiguration::isConfigured()
{
    return m_bConfigured;
}


cBridgeConfigData *cBridgeConfiguration::getConfigurationData()
{
    return m_pBridgeConfigData;
}


void cBridgeConfiguration::configXMLInfo(QString key)
{
    bool ok;

    if (m_ConfigXMLMap.contains(key))
    {
        ok = true;
        int cmd = m_ConfigXMLMap[key];
        switch (cmd)
        {
        case setDebugLevel:
            m_pBridgeConfigData->m_nDebugLevel = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setReferenceMeterIp:
            m_pBridgeConfigData->m_sIPReferenceMeter = m_pXMLReader->getValue(key);
            break;
        case setReferenceMeterPort:
            m_pBridgeConfigData->m_nPortReferenceMeter = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setSPICtrlDeviceName:
            m_pBridgeConfigData->m_sSPICtrlDeviceName = m_pXMLReader->getValue(key);
            break;
        case setSPIDataDeviceName:
            m_pBridgeConfigData->m_sSPIDataDeviceName = m_pXMLReader->getValue(key);
            break;
        case setRangeAuto:
            m_pBridgeConfigData->m_bRangeAuto = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setGrouping:
            m_pBridgeConfigData->m_bGrouping = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setIntegrationtime:
            m_pBridgeConfigData->m_nIntegrationtime = m_pXMLReader->getValue(key).toInt(&ok);
            break;
        case setVoltagerangeCount:
            m_pBridgeConfigData->m_nVoltagerangeCount = m_pXMLReader->getValue(key).toInt(&ok);
            for (int i = 0; i < m_pBridgeConfigData->m_nVoltagerangeCount; i++)
                m_ConfigXMLMap[QString("bridgeconf:referencemeter:voltagerange:code%1").arg(i+1)] = setVoltagerange1 + i;
            break;
        case setCurrentrangeCount:
            m_pBridgeConfigData->m_nCurrentrangeCount = m_pXMLReader->getValue(key).toInt(&ok);
            for (int i = 0; i < m_pBridgeConfigData->m_nCurrentrangeCount; i++)
                m_ConfigXMLMap[QString("bridgeconf:referencemeter:currentrange:code%1").arg(i+1)] = setCurrentrange1 + i;
            break;
        case setMeasuringmodeCount:
            m_pBridgeConfigData->m_nMeasuringmodeCount = m_pXMLReader->getValue(key).toInt(&ok);
            for (int i = 0; i < m_pBridgeConfigData->m_nMeasuringmodeCount; i++)
                m_ConfigXMLMap[QString("bridgeconf:referencemeter:measuringmode:code%1").arg(i+1)] = setMeasuringmode1 + i;
            break;
        default:
            if ((cmd >= setVoltagerange1) && (cmd < setVoltagerange1 + 32))
            {
                cmd -= setVoltagerange1;
                // it is command for setting a voltage range with selection code
                QStringList volRangeInfo = m_pXMLReader->getValue(key).split(",");
                m_pBridgeConfigData->m_VoltageRangeHash[volRangeInfo[0].toInt(&ok)] = volRangeInfo[1];
            }
            else
                if ((cmd >= setCurrentrange1) && (cmd < setCurrentrange1 + 32))
                {
                    cmd -= setCurrentrange1;
                    // it is command for setting a current range with selection code
                    QStringList curRangeInfo = m_pXMLReader->getValue(key).split(",");
                    m_pBridgeConfigData->m_CurrentRangeHash[curRangeInfo[0].toInt(&ok)] = curRangeInfo[1];
                }
            else
                    if ((cmd >= setMeasuringmode1) && (cmd < setMeasuringmode1 + 32))
                    {
                        cmd -= setMeasuringmode1;
                        // it is command for setting a measuring mode with selection code and related power meters
                        QString module, mode;
                        QStringList mmInfo = m_pXMLReader->getValue(key).split(",");
                        cModeSelect mSelect;

                        int mmnr = mmInfo.takeFirst().toInt(&ok);
                        if (ok)
                        {
                            while (mmInfo.count() > 0)
                            {
                                module = mmInfo.takeFirst();
                                mode = mmInfo.takeFirst();
                                mSelect.addMeasuringMode(module, mode);
                            }
                        }

                        m_pBridgeConfigData->m_MeasuringmodeHash[mmnr] = mSelect;
                    }
        }
        m_bConfigError |= !ok;
    }
    else
        m_bConfigError = true;
}


void cBridgeConfiguration::completeConfiguration(bool ok)
{
    m_bConfigured = (ok && !m_bConfigError);
    if (m_bConfigured)
        emit configXMLDone();
    else
        emit error(configError);
}


