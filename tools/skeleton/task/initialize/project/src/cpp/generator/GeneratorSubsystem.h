#ifndef GENERATORSUBSYSTEM_H_
#define GENERATORSUBSYSTEM_H_

#include "generator/base/BaseGeneratorSubsystem.h"

using namespace std;
using namespace Poco;

namespace Myriad {

class GeneratorSubsystem : public BaseGeneratorSubsystem
{
public:

    GeneratorSubsystem(NotificationCenter& notificationCenter, const vector<bool>& executeStages) :
        BaseGeneratorSubsystem(notificationCenter, executeStages)
    {
    }

    virtual ~GeneratorSubsystem()
    {
    }

protected:

    virtual void registerGenerators();
};

} // namespace Myriad

#endif /* GENERATORSUBSYSTEM_H_ */
