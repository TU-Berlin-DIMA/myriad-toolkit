// auto-generatad base generator config C++ file

#ifndef BASEGENERATORSUBSYSTEM_H_
#define BASEGENERATORSUBSYSTEM_H_

#include "generator/AbstractGeneratorSubsystem.h"

namespace Myriad {

class BaseGeneratorSubsystem: public AbstractGeneratorSubsystem
{
public:

    BaseGeneratorSubsystem(NotificationCenter& notificationCenter, const vector<bool>& executeStages) : 
        AbstractGeneratorSubsystem(notificationCenter, executeStages)
    {
    }

    virtual ~BaseGeneratorSubsystem()
    {
    }

protected:

    virtual void registerGenerators();
};

} // namespace Myriad

#endif /* BASEGENERATORSUBSYSTEM_H_ */
