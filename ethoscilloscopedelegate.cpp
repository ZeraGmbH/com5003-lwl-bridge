#include <math.h>
#include <QTcpSocket>

#include "ethoscilloscopedelegate.h"
#include "ethmeasuredelegate.h"


cETHOscilloscopeDelegate::cETHOscilloscopeDelegate(QTcpSocket *socket, cETHMeasureDelegate* measDelegate )
    :cETHCmdDelegate(socket), m_pMeasureDelegate(measDelegate)
{
    channelList << "ul1" << "ul2" << "ul3" << "il1" << "il2" << "il3";
    m_nChannel = 1; // default
}


cETHOscilloscopeDelegate::~cETHOscilloscopeDelegate()
{
}


void cETHOscilloscopeDelegate::execute()
{
    int index;

    index = m_nChannel - 1;

    switch (index)
    {
    case 0:
    case 1:
    case 2:
        m_fNorm = 19275.0 / m_pMeasureDelegate->getActualValue("UB");
        break;
    case 3:
    case 4:
    case 5:
        m_fNorm = 18204.0 / m_pMeasureDelegate->getActualValue("IB");
        break;
    }

    m_fPhaseAngle = m_pMeasureDelegate->getActualValue(QString("W%1").arg(channelList.at(index).toUpper()));

    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
    m_pSocket->write(QString("meas:osc1:%1?\n").arg(channelList.at(index)).toLatin1());
}


void cETHOscilloscopeDelegate::setChannel(int index)
{
    m_nChannel = index;
}


QVector<quint16> &cETHOscilloscopeDelegate::getOscillogram()
{
   return m_nOscillogram;
}


void cETHOscilloscopeDelegate::receiveAnswer()
{
    QStringList sampleStrings;
    QString answer;
    QString key, data;
    int pos;
    bool ok;

    data = answer = QString(m_pSocket->readLine());
    disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));

    pos = answer.lastIndexOf(":");
    key = answer.left(pos+1);
    data.remove(key);
    data.remove(';');
    data.remove('\n');

    sampleStrings = data.split(',');

    m_fOscillogram.clear();
    for (int i = 0; i < sampleStrings.count(); i++)
        m_fOscillogram.append(sampleStrings.at(i).toDouble(&ok));

    cmpOscillogram();

    emit finished();
}


void cETHOscilloscopeDelegate::cmpOscillogram()
{
    double dTRef = 1.0 / m_fOscillogram.size(); // period is 1.0 / number of samples
    double dTFG301 = 1.0 / 720.0; // dito
    double tPhase = m_fPhaseAngle * 720.0 / 360.0;

    int len;
    len = m_fOscillogram.count();
    for (int i = 0; i < len; i++)
        m_fOscillogram.append(m_fOscillogram.at(i));
    // we simply doubled the wave length for phase shifting

    m_nOscillogram.clear();

    for (int i = 0; i < 720; i++)
    {
        double tFG301 = i * dTFG301;
        double indexD = (tPhase+tFG301) / dTRef;
        int index = floor(indexD);

        double sample = m_fOscillogram.at(index)
                      + (m_fOscillogram.at(index+1) - m_fOscillogram.at(index)) * (indexD - index);

        sample *= m_fNorm; // normalized to voltage 19275 lsb or current 18204 lsb (frieling and otto desolution)
        m_nOscillogram.append((int)floor(sample +0.5));
    }

}
