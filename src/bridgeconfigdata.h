#ifndef BRIDGECONFIGURATIONDATA
#define BRIDGECONFIGURATIONDATA

#include <QString>
#include <QHash>


class cModeSelect
{
public:
    cModeSelect(){}
    cModeSelect(QString module, QString mode): m_sModuleName(module), m_sMeasmodeName(mode){}
    QString m_sModuleName;
    QString m_sMeasmodeName;
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
    quint8 m_nMeasuringmodeCrossCount;
    QHash<int, QString> m_VoltageRangeHash;
    QHash<int, QString> m_CurrentRangeHash;
    QHash<int, cModeSelect> m_MeasuringmodeHash;
    QHash<QString, QStringList> m_CrossreferenceModeHash;
};

#endif // BRIDGECONFIGURATIONDATA

