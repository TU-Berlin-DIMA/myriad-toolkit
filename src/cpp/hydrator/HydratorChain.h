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
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */

#ifndef HYDRATORCHAIN_H_
#define HYDRATORCHAIN_H_

#include "hydrator/RecordHydrator.h"

#include <Poco/AutoReleasePool.h>
#include <Poco/AutoPtr.h>
#include <vector>

using namespace Poco;
using namespace std;

namespace Myriad {

class BaseHydratorChain
{
public:

	enum OperationMode
	{
		SEQUENTIAL, RANDOM
	};

	BaseHydratorChain(OperationMode opMode, RandomStream& random) : _random(random), _opMode(opMode)
	{
	}

protected:

	void ensurePosition(I64u position) const;

	RandomStream& _random;

private:

	const OperationMode _opMode;
};

inline void BaseHydratorChain::ensurePosition(I64u position) const
{
	if (_opMode == RANDOM)
	{
		const_cast<BaseHydratorChain*>(this)->_random.atChunk(position);
	}
}

/**
 * Interface template for all HydratorChain.
 */
template<class RecordType>
class HydratorChain : public BaseHydratorChain
{
public:

	HydratorChain<RecordType>(OperationMode opMode, RandomStream& random) : BaseHydratorChain(opMode, random)
	{
	}

	virtual ~HydratorChain<RecordType>()
	{
	}

	/**
	 * Object hydrating function.
	 */
	virtual void operator()(AutoPtr<RecordType> recordPtr) const = 0;

	/**
	 * Invertible hydrator getter.
	 */
	template<typename T>
	const InvertibleHydrator<RecordType, T>& invertableHydrator(typename MethodTraits<RecordType, T>::Setter setter)
	{
		throw LogicException("The hydrator object associated with the specified setter is not invertible");
	}

protected:

    template<class HydratorType>
    void apply(HydratorType& hydrator, AutoPtr<RecordType>& recordPtr) const;
};

template<class RecordType> template<class HydratorType>
inline void HydratorChain<RecordType>::apply(HydratorType& hydrator, AutoPtr<RecordType>& recordPtr) const
{
    if (hydrator.enabled())
    {
        hydrator(recordPtr);
    }
    else
    {
        const_cast<HydratorChain<RecordType>*>(this)->_random.skip(hydrator.randomStreamArity());
    }
}

} // namespace Myriad

#endif /* HYDRATORCHAIN_H_ */
