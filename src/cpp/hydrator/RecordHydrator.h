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

#ifndef RECORDHYDRATOR_H_
#define RECORDHYDRATOR_H_

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad {

template<class RecordType> class RecordHydrator
{
public:

	RecordHydrator() : _enabled(true)
	{
	}

	virtual ~RecordHydrator()
	{
	}

	/**
	 * Object hydrating function (external PRNG).
	 */
	virtual void operator()(AutoPtr<RecordType> recordPtr) const = 0;

protected:

	bool _enabled;
};

template<class RecordType, class T> class InvertibleHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*ValueSetter)(const T&);

	InvertibleHydrator(ValueSetter valueSetter) :
		_valueSetter(valueSetter)
	{
	}

	virtual ~InvertibleHydrator()
	{
	}

	virtual const Interval<I64u> operator()(const T& x) const = 0;

protected:

	ValueSetter _valueSetter;
};

} // namespace Myriad

#endif /* RECORDHYDRATOR_H_ */
