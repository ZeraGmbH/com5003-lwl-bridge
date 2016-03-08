#ifndef ETHCMDSERIALIZER
#define ETHCMDSERIALIZER

#include <QObject>
#include <QList>

class cETHCmdDelegate;


class cETHCmdSerializer: public QObject
{
    Q_OBJECT

public:
    cETHCmdSerializer(){}
    ~cETHCmdSerializer(){}

    void execute(cETHCmdDelegate* delegate);

signals:
    void finished();

private:
    QList<cETHCmdDelegate*> ethCmdDelegateList;

private slots:
    void delegateReady();
};


#endif // ETHCMDSERIALIZER

