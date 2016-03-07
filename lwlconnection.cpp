#include <QTimer>
#include <QHash>

#include "lwlconnection.h"
#include "spiconnection.h"

cLWLConnection::cLWLConnection(cSPIConnection *spiconnection)
    :m_pSPIConnection(spiconnection)
{
    m_bconnected = false;

    m_pLWLLoadTimer = new QTimer(this);
    connect(m_pLWLLoadTimer, SIGNAL(timeout()), this, SLOT(readLWLInput()));
    m_pLWLLoadTimer->start(100); // our base time for lwl test is 100 ms
    lwlOutput.resize(lwlOutputDataLength); // we reserve max. used length
}


cLWLConnection::~cLWLConnection()
{
    disconnect(0,0,0,0); // disconnect everything
    m_pLWLLoadTimer->stop();
    delete m_pSPIConnection;
    delete m_pLWLLoadTimer;
}


bool cLWLConnection::isConnected()
{
    return m_bconnected;
}


QByteArray &cLWLConnection::getLWLInput()
{
    return lwlInput;
}


void cLWLConnection::sendActualValues(QHash<QString, double *> &actualValuesHash)
{
    // we set actual value to fg301 now
    QString s;

    int pos = 0;

    s = QString("%1").arg(0.0, 8, 'g', 3); // dc values = 0
    for (int i = 0; i < 6; i++, pos+=8)
        lwlOutput.replace(pos, 8, s.toLatin1());

    QList<QString> voltRMSList{"UL1", "UL2", "UL3"};

    for (int i = 0; i < voltRMSList.count(); i++, pos+=8)
    {
        s = QString("%1").arg(*(actualValuesHash[voltRMSList.at(i)]), 8, 'g', 3);
        lwlOutput.replace(pos, 8, s.toLatin1());
    }

    QList<QString> currentRMSList{"IL1", "IL2", "IL3"};
    for (int i = 0; i < currentRMSList.count(); i++, pos+=10)
    {
        s = QString("%1").arg(*(actualValuesHash[currentRMSList.at(i)]), 10, 'g', 5);
        lwlOutput.replace(pos, 10, s.toLatin1());
    }

    QList<QString> angleList{"WUL1", "WUL2", "WUL3","WIL1", "WIL2", "WIL3"};
    for (int i = 0; i < angleList.count(); i++, pos+=6)
    {
        s = QString("%1").arg(*(actualValuesHash[angleList.at(i)]), 6, 'g', 2);
        lwlOutput.replace(pos, 6, s.toLatin1());
    }

    QList<QString> actPowerList{"P1", "P2", "P3"};
    for (int i = 0; i < actPowerList.count(); i++, pos+=12)
    {
        s = QString("%1").arg(*(actualValuesHash[actPowerList.at(i)]), 12, 'g', 5);
        lwlOutput.replace(pos, 12, s.toLatin1());
    }

    QList<QString> reactPowerList{"Q1", "Q2", "Q3"};
    for (int i = 0; i < reactPowerList.count(); i++, pos+=12)
    {
        s = QString("%1").arg(*(actualValuesHash[reactPowerList.at(i)]), 12, 'g', 5);
        lwlOutput.replace(pos, 12, s.toLatin1());
    }

    double PS = 0.0;
    for (int i = 0; i < actPowerList.count(); i++)
        PS += *(actualValuesHash[actPowerList.at(i)]);

    lwlOutput.replace(pos, 12, QString("%1").arg(PS, 12, 'g', 5).toLatin1());
    pos+=12;

    double QS = 0.0;

    for (int i = 0; i < reactPowerList.count(); i++)
        QS += *(actualValuesHash[reactPowerList.at(i)]);

    lwlOutput.replace(pos, 12, QString("%1").arg(QS, 12, 'g', 5).toLatin1());
    pos+=12;

    lwlOutput.replace(pos, 6, QString("%1").arg(*(actualValuesHash["F"]), 6, 'g', 2).toLatin1());
    pos+=6;

    lwlOutput.replace(pos, 6, QString("%1").arg(*(actualValuesHash["UB"]), 6, 'g', 2).toLatin1());
    pos+=6;

    if ( (*(actualValuesHash["IB"]) < 1.0))
    {
        s = QString("%1").arg(*(actualValuesHash["IB"]), 6, 'g', 3);
        lwlOutput.replace(pos, 6, s.toLatin1());
    }
    else
    {
        s = QString("%1").arg(*(actualValuesHash["IB"]), 6, 'g', 2);
        lwlOutput.replace(pos, 6, s.toLatin1());
    }
    pos+=6;

    // up to here we set actual values, voltage and current range information

    QByteArray ba;

    ba.append(char(0)); // sync byte for commands (adr = 253)
    ba.append(char(0));

    ba.append(char(0));
    ba.append(char(0)); // pztype information, imlementend for plugNplay but never used

    lwlOutput.replace(pos, 4, ba);
    pos+=4;

    QList<QString> appPowerList{"S1", "S2", "S3"};
    for (int i = 0; i < appPowerList.count(); i++, pos+=12)
    {
        s = QString("%1").arg(*(actualValuesHash[appPowerList.at(i)]), 12, 'g', 5);
        lwlOutput.replace(pos, 12, s.toLatin1());
    }

    double SS = 0.0;

    for (int i = 0; i < appPowerList.count(); i++)
        SS += *(actualValuesHash[appPowerList.at(i)]);

    lwlOutput.replace(pos, 12, QString("%1").arg(SS, 12, 'g', 5).toLatin1());
    pos+=12; // should be 304 now

    appendChksum(304);
    writeLWLOutput(306); // now we write the output to fpga memory

}


void cLWLConnection::sendOscillogram(QVector<quint16> &oscillogram)
{
    char data[2];
    for (int i = 0; i < 720; i++)
    {
        data[0] = (oscillogram.at(i) >> 8) & 0xff;
        data[1] = oscillogram.at(i) & 0xff;
        lwlOutput.replace(256+i*2, 2, &data[0], 2);
    }
    writeLWLOutput(lwlOutputDataLength);
}


void cLWLConnection::sendCmdRecognized(bool on)
{
    QByteArray ba;

    if (on)
        ba.append(char(1));
    else
        ba.append(char(0));

    lwlOutput.replace(253, 1, ba);
    // appendChksum(304);
    writeLWLOutput(256); // we only write 256 bytes , otherwise we destroy oscillogram
}


void cLWLConnection::setDisconnectCount()
{
    m_nDisconnectCount = 80; // we accept up to 80*50ms before we encounter disconnection
}


void cLWLConnection::writeLWLOutput(int len)
{
    if (m_bconnected)
    {
        m_pSPIConnection->writeSPI(lwlOutput, lwlOutputAdress, len); // if connected we write output
    }
}


void cLWLConnection::readLWLInput()
{
    QByteArray pLWLTestInput;

    if (m_pSPIConnection->readSPI(pLWLTestInput, lwlInputAdress, lwlInputDataLength))
    {
        quint8 chksum;
        quint8 *lwlData;
        bool dataValid;
        int i;

        chksum = 0;
        lwlData = (quint8*) pLWLTestInput.data();

        for (i = 0; i < lwlInputDataLength-1; i++)
            chksum += lwlData[i];

        dataValid = (chksum == lwlData[i]);

        if (m_bconnected)
        {
            if (!dataValid) // we send that we are disconnected in case that counter has reached zero
            {
                m_nDisconnectCount--;
                if (m_nDisconnectCount == 0)
                {
                    m_bconnected = false;
                    emit disconnected();
                }
            }
            else
            {
                int j;
                int len;

                len = lwlInputDataLength-1; // we test without chksum
                for (j = 0; j < len; j++) // lets test if we have new data
                {
                   if ( (pLWLTestInput)[j] != lwlInput.at(j) )
                       break;
                }

                if (j < len) // if we encountered 1 different byte then data has changed
                {
                    lwlInput = pLWLTestInput;
                    QByteArray ba;
                    ba.append(~lwlData[i]);
                    pLWLTestInput.replace(lwlInputDataLength, 1, ba);
                    setDisconnectCount();
                    m_pSPIConnection->writeSPI(pLWLTestInput, lwlInputAdress, lwlInputDataLength); // we make chksum invalid
                    emit dataAvail(); // we only send signal if new data is avail
                }
            }
        }
        else
            if (dataValid)
            {
                lwlInput = pLWLTestInput;
                setDisconnectCount();
                m_bconnected = true;
                emit connected();
            }
    }
}


void cLWLConnection::appendChksum(int len)
{
    quint16 chksum;

    chksum = 0;
    for (int i = 0; i < len; i++)
        chksum += lwlOutput[i];

    QByteArray ba;
    ba.append((chksum >> 8) & 0xff);
    ba.append(chksum & 0xff);

    lwlOutput.replace(len, 2, ba);
}


