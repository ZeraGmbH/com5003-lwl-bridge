#ifndef ETHOSCILLOSCOPEDELEGATE
#define ETHOSCILLOSCOPEDELEGATE

#include <QObject>
#include <QStringList>
#include <QString>
#include <QVector>

#include "ethcmddelegate.h"

class cETHMeasureDelegate;

class cETHOscilloscopeDelegate: public cETHCmdDelegate
{
    Q_OBJECT
public:
    cETHOscilloscopeDelegate(QTcpSocket *socket, cETHMeasureDelegate *measDelegate);
    virtual ~cETHOscilloscopeDelegate();

    virtual void execute();
    void setChannel(int index);
    QVector<qint16>& getOscillogram();

protected slots:
    virtual void receiveAnswer();

private:
    cETHMeasureDelegate *m_pMeasureDelegate;
    QList<QString> channelList;
    int m_nChannel;
    double m_fNorm;
    QVector<double> m_fOscillogram; // the oscillogram read
    QVector<qint16> m_nOscillogram; // the oscillogram for fg301
    double m_fPhaseAngle;

    void cmpOscillogram();


};

#endif // ETHOSCILLOSCOPEDELEGATE

