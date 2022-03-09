#ifndef DIALOGFACTORY_H
#define DIALOGFACTORY_H

#include "../auxiliary/Constants.h"

QT_FORWARD_DECLARE_CLASS(DialogBase)

class DialogFactory
{
public:
    static DialogFactory *instance();
    ~DialogFactory();

    DialogBase* instantiate(const Constants::MeasureType&);
    DialogBase* instantiate(const QString&);

private:
    DialogFactory() = default;
    static DialogFactory *pInstance;
};

#endif // DIALOGFACTORY_H
