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

#ifndef SIMPLEVALUEGETTER_H_
#define SIMPLEVALUEGETTER_H_

#include "reflection/getter/ValueGetter.h"

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad {

template<class RecordType, typename FieldType> class SimpleValueGetter : public ValueGetter<RecordType, FieldType>
{
public:

	typedef const FieldType& (RecordType::*SimpleValueGetterMethod)() const;

	SimpleValueGetter(SimpleValueGetterMethod valueGetter) :
		_valueGetter(valueGetter)
	{
	}

	virtual ~SimpleValueGetter()
	{
	}

	virtual const FieldType& operator()(AutoPtr<RecordType> recordPtr) const;

private:

	SimpleValueGetterMethod _valueGetter;
};

template<class RecordType, typename FieldType> inline const FieldType& SimpleValueGetter<RecordType, FieldType>::operator()(AutoPtr<RecordType> recordPtr) const
{
	return (recordPtr->*_valueGetter)();
}

} // namespace Myriad

#endif /* SIMPLEVALUEGETTER_H_ */
