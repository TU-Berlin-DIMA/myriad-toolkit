// auto-generatad base generator config C++ file

#ifndef BASEGENERATORCONFIG_H_
#define BASEGENERATORCONFIG_H_

#include "config/AbstractGeneratorConfig.h"

namespace Myriad {

class BaseGeneratorConfig: public AbstractGeneratorConfig
{
public:

    BaseGeneratorConfig(GeneratorPool& generatorPool) : AbstractGeneratorConfig(generatorPool)
    {
    }

protected:

    virtual void configureFunctions()
    {
    }

    virtual void configurePartitioning()
    {
    }

    virtual void configureSets()
    {
    }
};

} // namespace Myriad

#endif /* BASEGENERATORCONFIG_H_ */
