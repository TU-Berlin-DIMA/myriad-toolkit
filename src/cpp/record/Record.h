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

#include "core/types.h"

#include <map>
#include <string>
#include <Poco/Any.h>
#include <Poco/DynamicAny.h>
#include <Poco/RefCountedObject.h>

using namespace std;
using namespace Poco;

#ifndef RECORD_H_
#define RECORD_H_

namespace Myriad {

// forward declarations
class RecordGenerator;
template<class RecordType> class HydratorChain;

template<class RecordType> struct RecordTraits
{
	typedef RecordGenerator GeneratorType;
	typedef HydratorChain<RecordType> HydratorChainType;
};

class Record: public Poco::RefCountedObject
{
public:

	I64u genID() const;
	void genID(const I64u v);

private:

	ID	meta_genid;
};

inline I64u Record::genID() const
{
	return meta_genid;
}

inline void Record::genID(const I64u v)
{
	meta_genid = v;
}

} // namespace Myriad

#endif /* RECORD_H_ */
