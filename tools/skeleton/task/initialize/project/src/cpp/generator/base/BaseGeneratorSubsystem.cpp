// auto-generatad base generator config C++ file

#include "generator/base/BaseGeneratorSubsystem.h"

using namespace std;
using namespace Poco;

namespace Myriad {

// the initial stage ID should always be zero
I32u RecordGenerator::Stage::NEXT_STAGE_ID = 0;

// register the valid stages for the Myriad generator extension
RecordGenerator::StageList initList()
{
    RecordGenerator::StageList tmp;

    return tmp;
}

const RecordGenerator::StageList RecordGenerator::STAGES(initList());

// register the record sequence generators
void BaseGeneratorSubsystem::registerGenerators()
{
}

} // namespace Myriad
