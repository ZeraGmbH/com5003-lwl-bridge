#ifndef ETHMEASUREDELEGATE
#define ETHMEASUREDELEGATE

#include <QObject>
#include <QHash>
#include <QTimer>

#include "ethcmddelegate.h"


class cETHMeasureDelegate: public cETHCmdDelegate
{
    Q_OBJECT
public:
    cETHMeasureDelegate(QTcpSocket *socket, int ti);
    virtual ~cETHMeasureDelegate();

    virtual void execute();
    QHash<QString, double*>& getActualValues();
    void setAngleReference(int index);

protected slots:
    virtual void receiveAnswer();

private:
    int m_nTi;
    QTimer toTimer;
    QStringList m_sCmdList;
    QHash<QString, double*> m_ActualValuesHash;
    QHash<QString, double*> m_ActualValuesDecodeHash;
    QHash<QString, QString> m_ActualDFTDecodeHash;
    QString m_sReferenceAngle;

    quint8 m_nAnswerCount;

private slots:
    void mValTimeout();
};

#endif // ETHMEASUREDELEGATE

