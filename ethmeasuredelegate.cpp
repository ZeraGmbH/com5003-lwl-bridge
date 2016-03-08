#include <QTcpSocket>

#include <useratan.h>

#include "ethmeasuredelegate.h"


cETHMeasureDelegate::cETHMeasureDelegate()
{
    // we init the meas command list once
    m_sCmdList.append(QString("meas:rms1?\n"));
    m_sCmdList.append(QString("meas:dft1?\n"));
    m_sCmdList.append(QString("meas:pow1?\n"));
    m_sCmdList.append(QString("meas:pow2?\n"));
    m_sCmdList.append(QString("meas:pow3?\n"));
    m_sCmdList.append(QString("meas:rng1:f?\n"));
    m_sCmdList.append(QString("sens:rng1:ul1:rang?\n")); // we handle the range queries at the same time as measurement
    m_sCmdList.append(QString("sens:rng1:il1:rang?\n"));
    m_sCmdList.append(QString("*opc?\n"));

    // a hash holds our actual values or intermediate values;

    m_ActualValuesHash["UL1"] = new double;
    m_ActualValuesHash["UL2"] = new double;
    m_ActualValuesHash["UL3"] = new double;
    m_ActualValuesHash["IL1"] = new double;
    m_ActualValuesHash["IL2"] = new double;
    m_ActualValuesHash["IL3"] = new double;

    m_ActualValuesHash["WUL1"] = new double;
    m_ActualValuesHash["WUL2"] = new double;
    m_ActualValuesHash["WUL3"] = new double;
    m_ActualValuesHash["WIL1"] = new double;
    m_ActualValuesHash["WIL2"] = new double;
    m_ActualValuesHash["WIL3"] = new double;

    m_ActualValuesHash["P1"] = new double;
    m_ActualValuesHash["P2"] = new double;
    m_ActualValuesHash["P3"] = new double;
    m_ActualValuesHash["Q1"] = new double;
    m_ActualValuesHash["Q2"] = new double;
    m_ActualValuesHash["Q3"] = new double;
    m_ActualValuesHash["S1"] = new double;
    m_ActualValuesHash["S2"] = new double;
    m_ActualValuesHash["S3"] = new double;

    m_ActualValuesHash["F"] = new double;

    m_ActualValuesHash["UB"] = new double;
    m_ActualValuesHash["IB"] = new double;

    m_ActualValuesDecodeHash["rms1:ul1:[V]:"] = m_ActualValuesHash["UL1"];
    m_ActualValuesDecodeHash["rms1:ul2:[V]:"] = m_ActualValuesHash["UL2"];
    m_ActualValuesDecodeHash["rms1:ul3:[V]:"] = m_ActualValuesHash["UL3"];
    m_ActualValuesDecodeHash["rms1:il1:[A]:"] = m_ActualValuesHash["IL1"];
    m_ActualValuesDecodeHash["rms1:il2:[A]:"] = m_ActualValuesHash["IL2"];
    m_ActualValuesDecodeHash["rms1:il3:[A]:"] = m_ActualValuesHash["IL3"];

    m_ActualDFTDecodeHash["dft1:ul1:[V]:"] = "WUL1";
    m_ActualDFTDecodeHash["dft1:ul2:[V]:"] = "WUL2";
    m_ActualDFTDecodeHash["dft1:ul3:[V]:"] = "WUL3";
    m_ActualDFTDecodeHash["dft1:il1:[A]:"] = "WIL1";
    m_ActualDFTDecodeHash["dft1:il2:[A]:"] = "WIL2";
    m_ActualDFTDecodeHash["dft1:il3:[A]:"] = "WIL3";

    m_ActualValuesDecodeHash["pow1:p1:[W]:"] = m_ActualValuesHash["P1"];
    m_ActualValuesDecodeHash["pow1:p2:[W]:"] = m_ActualValuesHash["P2"];
    m_ActualValuesDecodeHash["pow1:p3:[W]:"] = m_ActualValuesHash["P3"];

    m_ActualValuesDecodeHash["pow2:q1:[Var]:"] = m_ActualValuesHash["Q1"];
    m_ActualValuesDecodeHash["pow2:q2:[Var]:"] = m_ActualValuesHash["Q2"];
    m_ActualValuesDecodeHash["pow2:q3:[Var]:"] = m_ActualValuesHash["Q3"];

    m_ActualValuesDecodeHash["pow3:s1:[VA]:"] = m_ActualValuesHash["S1"];
    m_ActualValuesDecodeHash["pow3:s2:[VA]:"] = m_ActualValuesHash["S2"];
    m_ActualValuesDecodeHash["pow3:s3:[VA]:"] = m_ActualValuesHash["S3"];

    m_ActualValuesDecodeHash["rng1:f:[Hz]:"] = m_ActualValuesHash["F"];

    m_sReferenceAngle = "UL1"; // default
}


cETHMeasureDelegate::~cETHMeasureDelegate()
{
    QList<QString> keyList;
    double* doublePtr;

    keyList = m_ActualValuesHash.keys();
    for (int i = 0; i < keyList.count(); i++)
    {
        doublePtr = m_ActualValuesHash[keyList.at(i)];
        delete doublePtr;
    }

}


void cETHMeasureDelegate::execute()
{
    // we send all needed commands at once
    connect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
    for (int i = 0; m_sCmdList.count(); i++)
        m_pSocket->write(m_sCmdList.at(i).toLatin1());
}


QHash<QString, double *>& cETHMeasureDelegate::getActualValues()
{
    return m_ActualValuesHash;
}


void cETHMeasureDelegate::setAngleReference(int index)
{
    switch (index)
    {
    case 0:
        m_sReferenceAngle = "WUL1";
        break;
    case 1:
        m_sReferenceAngle = "WUL2";
        break;
    case 2:
        m_sReferenceAngle = "WUL3";
        break;
    case 3:
        m_sReferenceAngle = "WIL1";
        break;
    case 4:
        m_sReferenceAngle = "WIL2";
        break;
    case 5:
        m_sReferenceAngle = "WIL3";
        break;
    }
}


void cETHMeasureDelegate::receiveAnswer()
{
    while (m_pSocket->canReadLine())
    {
        QString answer;

        answer = QString(m_pSocket->readLine());
        answer.remove('\n');

        if (answer == QString("+1"))
        {
            // it was the answer to the last last command (opc)
            double refAngle;
            refAngle = *m_ActualValuesHash[m_sReferenceAngle];
            *(m_ActualValuesHash["WUL1"]) = *(m_ActualValuesHash["WUL1"]) - refAngle;
            *(m_ActualValuesHash["WUL2"]) = *(m_ActualValuesHash["WUL2"]) - refAngle;
            *(m_ActualValuesHash["WUL3"]) = *(m_ActualValuesHash["WUL3"]) - refAngle;
            *(m_ActualValuesHash["WIL1"]) = *(m_ActualValuesHash["WIL1"]) - refAngle;
            *(m_ActualValuesHash["WIL2"]) = *(m_ActualValuesHash["WIL2"]) - refAngle;
            *(m_ActualValuesHash["WIL3"]) = *(m_ActualValuesHash["WIL3"]) - refAngle;

            emit finished();
        }

        else
        {
            if (answer.count(';') == 0)
            {
                // so we have an answer to voltage or current range query

                double rngValue;
                bool ok;

                if (answer.count('V') > 0)
                {
                    answer.replace("V","");
                    rngValue = answer.toDouble(&ok);
                    *(m_ActualValuesHash["UB"]) = rngValue;
                }
                else
                {
                    double scale;

                    answer.replace("A","");
                    if (answer.contains('m') > 0)
                    {
                        answer.replace("m","");
                        scale = 0.001;
                    }
                    else
                        scale = 1.0;

                    rngValue = answer.toDouble(&ok) * scale;
                }
            }

            else
            {
                QStringList sl;

                sl = answer.split(';');

                for (int i = 0; i < sl.count(); i++)
                {
                    QString pString;
                    QString key, data;
                    int pos;
                    bool ok;

                    pString = sl.at(i);

                    pos = pString.lastIndexOf(":");
                    key = pString.leftJustified(pos);
                    data = pString.remove(key);

                    if (m_ActualDFTDecodeHash.contains(key))
                    {
                        // we have found a key that was expected for a dft value so we have to compute the angle
                        QStringList sl;
                        sl = data.split(',');
                        *(m_ActualValuesDecodeHash[m_ActualDFTDecodeHash[key] ]) = userAtan(sl.at(1).toDouble(&ok), sl.at(0).toDouble(&ok));

                    }
                    if (m_ActualValuesDecodeHash.contains(key))
                    {
                        *(m_ActualValuesDecodeHash[key]) = data.toDouble(&ok);
                    }
                }
            }
        }
    }

    disconnect(m_pSocket, SIGNAL(readyRead()), this, SLOT(receiveAnswer()));
}
