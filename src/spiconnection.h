#ifndef SPICONNECTION
#define SPICONNECTION

#include <QByteArray>

class cSPIConnection
{
public:
    cSPIConnection(){}
    ~cSPIConnection(){}

    bool writeSPI(QByteArray& Output, quint32 OutputAdress, quint32 len);
    bool readSPI(QByteArray& Input, quint32 InputAdress, quint32 len);
};

#endif // SPICONNECTION

