#include "generator/GeneratorSubsystem.h"

using namespace std;
using namespace Poco;

namespace Myriad {

void GeneratorSubsystem::registerGenerators()
{
    BaseGeneratorSubsystem::registerGenerators();

    // register generators with the autorelease pool
}

} // namespace Myriad
