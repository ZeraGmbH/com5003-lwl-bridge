#include "ethcmdserializer.h"
#include "ethcmddelegate.h"

void cETHCmdSerializer::execute(cETHCmdDelegate *delegate)
{
    bool empty;

    empty = ethCmdDelegateList.isEmpty();
    ethCmdDelegateList.append(delegate);
    if (empty)
    {
        connect(delegate, SIGNAL(finished()), this, SLOT(delegateReady()));
        delegate->execute();
    }
}


void cETHCmdSerializer::delegateReady()
{
    cETHCmdDelegate *delegate;
    delegate =  ethCmdDelegateList.takeFirst();
    disconnect(delegate, 0, 0, 0);

    if (ethCmdDelegateList.isEmpty())
        return;
    else
    {
        connect(delegate, SIGNAL(finished()), this, SLOT(delegateReady()));
        ethCmdDelegateList.at(0)->execute();
    }
}
