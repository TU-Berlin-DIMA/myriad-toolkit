#ifndef GENERATORCONFIG_H_
#define GENERATORCONFIG_H_

#include "config/base/BaseGeneratorConfig.h"

namespace Myriad {

class GeneratorConfig: public BaseGeneratorConfig
{
public:

    GeneratorConfig(GeneratorPool& generatorPool) : BaseGeneratorConfig(generatorPool)
    {
    }

protected:

    virtual void configureFunctions()
    {
        BaseGeneratorConfig::configureFunctions();
        // override or add functions here
    }

    virtual void configurePartitioning()
    {
        BaseGeneratorConfig::configurePartitioning();
        // override or add partitioning config here
    }

    virtual void configureSets()
    {
        BaseGeneratorConfig::configureSets();
        // override or add enumerated sets here
    }
};

} // namespace Myriad

#endif /* GENERATORCONFIG_H_ */
