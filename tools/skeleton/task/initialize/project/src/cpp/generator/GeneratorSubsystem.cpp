/*
 * Copyright 2010-2011 DIMA Research Group, TU Berlin
 * 
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 *     http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#include "generator/GeneratorSubsystem.h"
// include generator headers

using namespace std;
using namespace Poco;

namespace Myriad {

// the initial stage ID should always be zero
I32u RecordGenerator::Stage::NEXT_STAGE_ID = 0;

// register the valid stages for the Myriad generator extension
RecordGenerator::StageList initList()
{
	RecordGenerator::StageList tmp;

	tmp.push_back(RecordGenerator::Stage("default"));

	return tmp;
}

const RecordGenerator::StageList RecordGenerator::STAGES(initList());

void GeneratorSubsystem::registerGenerators()
{
	// register generators with the autorelease pool
}

} // namespace Myriad
