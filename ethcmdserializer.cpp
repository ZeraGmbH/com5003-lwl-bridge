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
    delegate =  ethCmdDelegateList.takeFirst(); // we remove this delegate from list
    disconnect(delegate, 0, 0, 0); // and disconnect all signals

    if (ethCmdDelegateList.isEmpty())
        return;
    else
    {
        delegate =  ethCmdDelegateList.first(); // we fetch the next delegate
        connect(delegate, SIGNAL(finished()), this, SLOT(delegateReady()));
        delegate->execute();
    }
}
