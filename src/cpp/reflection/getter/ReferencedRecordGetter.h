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

#ifndef REFERENCEDRECORDGETTER_H_
#define REFERENCEDRECORDGETTER_H_

#include "reflection/getter/ValueGetter.h"

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad {

template<class RecordType, typename FieldType> class ReferencedRecordGetter : public ValueGetter<RecordType, AutoPtr<FieldType> >
{
public:

	typedef const AutoPtr<FieldType>& (RecordType::*RecordGetterMethod)() const;

	ReferencedRecordGetter(RecordGetterMethod objectGetter) :
		_objectGetter(objectGetter)
	{
	}

	virtual ~ReferencedRecordGetter()
	{
	}

	virtual const AutoPtr<FieldType>& operator()(AutoPtr<RecordType> recordPtr) const;

private:

	RecordGetterMethod _objectGetter;
};

template<class RecordType, typename FieldType> inline const AutoPtr<FieldType>& ReferencedRecordGetter<RecordType, FieldType>::operator()(AutoPtr<RecordType> recordPtr) const
{
	return (recordPtr->*_objectGetter)();
}

} // namespace Myriad

#endif /* REFERENCEDRECORDGETTER_H_ */
