#include <math.h>
#include <QTcpSocket>

#include "ethoscilloscopedelegate.h"


cETHOscilloscopeDelegate::cETHOscilloscopeDelegate(QTcpSocket *socket)
    :cETHCmdDelegate(socket)
{
    m_fVoltageRange = 480.0; // default settings
    m_fCurrentRange = 160.0;

    m_sChannel = "ul1"; // default
}


cETHOscilloscopeDelegate::~cETHOscilloscopeDelegate()
{
}


void cETHOscilloscopeDelegate::execute()
{
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
    m_pSocket->write(QString("meas:osc1:%1?\n").arg(m_sChannel).toLatin1());
}


void cETHOscilloscopeDelegate::setChannel(int index)
{
    switch (index)
    {
    case 0:
        m_sChannel = "ul1";
        m_fNorm = 19275.0 / m_fVoltageRange;
        break;
    case 1:
        m_sChannel = "ul2";
        m_fNorm = 19275.0 / m_fVoltageRange;
        break;
    case 2:
        m_sChannel = "ul3";
        m_fNorm = 19275.0 / m_fVoltageRange;
        break;
    case 3:
        m_sChannel = "il1";
        m_fNorm = 18204.0 / m_fCurrentRange;
        break;
    case 4:
        m_sChannel = "il2";
        m_fNorm = 18204.0 / m_fCurrentRange;
        break;
    case 5:
        m_sChannel = "il3";
        m_fNorm = 18204.0 / m_fCurrentRange;
        break;
    }
}


QVector<quint16> &cETHOscilloscopeDelegate::getOscillogram()
{
   return m_nOscillogram;
}


void cETHOscilloscopeDelegate::setRange(double vRange, double cRange)
{
    m_fVoltageRange = vRange;
    m_fCurrentRange = cRange;
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
    data = data.remove(key);

    sampleStrings = data.split(',');

    m_fOscillogram.clear();
    for (int i = 0; i < sampleStrings.count(); i++)
        m_fOscillogram.append(sampleStrings.at(i).toDouble(&ok));

    cmpOscillogram();

    emit finished();
}


void cETHOscilloscopeDelegate::cmpOscillogram()
{
    double dTRef = 1.0 / m_fOscillogram.size();
    double dTFG301 = 1.0 / 720.0;

    m_nOscillogram.clear();

    for (int i = 0; i < 720; i++)
    {
        double tFG301 = i * dTFG301;
        double indexD = tFG301 / dTRef;
        int index = floor(indexD);

        double sample = m_fOscillogram.at(index)
                      + (m_fOscillogram.at(index+1) - m_fOscillogram.at(index)) * (indexD - index);

        sample *= m_fNorm; // normalized to voltage or current range * 19275 or 18204 (frieling and otto desolution)
        m_nOscillogram.append((int)floor(sample +0.5));
    }

}
