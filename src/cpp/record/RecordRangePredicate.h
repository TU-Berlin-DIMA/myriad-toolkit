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

#ifndef RECORDRANGEPREDICATE_H_
#define RECORDRANGEPREDICATE_H_

#include "core/types.h"
#include "record/Record.h"

namespace Myriad {

/**
 *
 */
template<class RecordType> class RecordRangePredicate: public RecordHydrator<RecordType>
{
public:

	RecordRangePredicate() :
		_intervals(new Interval<I64u>[32])
	{
	}

	virtual ~RecordRangePredicate()
	{
		delete[] _intervals;
	}

	/**
	 * @TODO: implement generic logic or throw a runtime exception here
	 */
	template<I16u fid> Interval<I64u> get()
	{
		return Interval<I64u>(0, 0);
	}

private:

	Interval<I64u>* _intervals;
};

}  // namespace Myriad

#endif /* RECORDRANGEPREDICATE_H_ */
