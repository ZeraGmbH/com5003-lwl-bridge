#ifndef BRIDGECONFIGURATIONDATA
#define BRIDGECONFIGURATIONDATA

#include <QString>
#include <QStringList>
#include <QHash>


class cModeSelect
{
public:
    cModeSelect(){}

    void addMeasuringMode(QString module, QString mode)
        {m_sModuleNameList.append(module);m_sMeasmodeNameList.append(mode);}

    QStringList m_sModuleNameList;
    QStringList m_sMeasmodeNameList;
};


struct cBridgeConfigData
{
    quint8 m_nDebugLevel;
    QString m_sIPReferenceMeter;
    quint16 m_nPortReferenceMeter;
    QString m_sSPICtrlDeviceName;
    QString m_sSPIDataDeviceName;
    int m_bRangeAuto;
    int m_bGrouping;
    int m_nIntegrationtime;
    quint8 m_nVoltagerangeCount;
    quint8 m_nCurrentrangeCount;
    quint8 m_nMeasuringmodeCount;
    QHash<int, QString> m_VoltageRangeHash;
    QHash<int, QString> m_CurrentRangeHash;
    QHash<int, cModeSelect> m_MeasuringmodeHash;
};

#endif // BRIDGECONFIGURATIONDATA

