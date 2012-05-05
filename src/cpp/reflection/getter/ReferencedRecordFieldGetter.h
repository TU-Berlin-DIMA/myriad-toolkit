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

#ifndef REFERENCEDRECORDFIELDGETTER_H_
#define REFERENCEDRECORDFIELDGETTER_H_

#include "reflection/getter/ValueGetter.h"

#include <Poco/AutoPtr.h>

using namespace Poco;

namespace Myriad {

template<class RecordType, class RefRecordType, class FieldType> class ReferencedRecordFieldGetter : public ValueGetter<RecordType, FieldType>
{
public:

	typedef const AutoPtr<RefRecordType>& (RecordType::*RecordGetterMethod)() const;
	typedef const FieldType& (RefRecordType::*FieldGetterMethod)() const;

	ReferencedRecordFieldGetter(RecordGetterMethod refRecordGetter, FieldGetterMethod fieldGetter) :
		_refRecordGetter(refRecordGetter),
		_fieldGetter(fieldGetter)
	{
	}

	virtual ~ReferencedRecordFieldGetter()
	{
	}

	virtual const FieldType& operator()(AutoPtr<RecordType> recordPtr) const;

private:

	RecordGetterMethod _refRecordGetter;
	FieldGetterMethod _fieldGetter;
};

template<class RecordType, class RefRecordType, class FieldType> inline const FieldType& ReferencedRecordFieldGetter<RecordType, RefRecordType, FieldType>::operator()(AutoPtr<RecordType> recordPtr) const
{
	return ((recordPtr->*_refRecordGetter)()->*_fieldGetter)();
}

} // namespace Myriad

#endif /* REFERENCEDRECORDFIELDGETTER_H_ */
