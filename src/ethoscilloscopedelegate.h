#ifndef ETHOSCILLOSCOPEDELEGATE
#define ETHOSCILLOSCOPEDELEGATE

#include <QObject>
#include <QStringList>
#include <QString>
#include <QVector>

#include "ethcmddelegate.h"

class cETHOscilloscopeDelegate: public cETHCmdDelegate
{
    Q_OBJECT
public:
    cETHOscilloscopeDelegate();
    virtual ~cETHOscilloscopeDelegate();

    virtual void execute();
    void setChannel(int index);
    QVector<quint16>& getOscillogram();
    void setRange(double vRange, double cRange);


protected slots:
    virtual void receiveAnswer();

private:
    QString m_sChannel;
    double m_fNorm;
    QVector<double> m_fOscillogram; // the oscillogram read
    QVector<quint16> m_nOscillogram; // the oscillogram for fg301
    double m_fVoltageRange;
    double m_fCurrentRange;

    void cmpOscillogram();
};

#endif // ETHOSCILLOSCOPEDELEGATE

