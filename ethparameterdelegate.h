#ifndef ETHPARAMETERDELEGATE
#define ETHPARAMETERDELEGATE

#include <QList>

#include "ethcmddelegate.h"


class cETHParameterDelegate: public cETHCmdDelegate
{
    Q_OBJECT

public:
    cETHParameterDelegate(){}
    virtual ~cETHParameterDelegate(){}

    virtual void setCmdList(QList<QString> sl);
    virtual void execute();

protected slots:
    virtual void receiveAnswer();

private:
    QList<QString> m_sCmdList;
};

#endif // ETHPARAMETERDELEGATE

