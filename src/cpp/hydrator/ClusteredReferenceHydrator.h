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

#ifndef CLUSTEREDREFERENCEHYDRATOR_H_
#define CLUSTEREDREFERENCEHYDRATOR_H_

#include "generator/InvalidRecordException.h"
#include "hydrator/RandomRecordHydrator.h"
#include "math/random/RandomStream.h"

namespace Myriad {

template<class RecordType, class RefRecordType> class ClusteredReferenceHydrator : public RecordHydrator<RecordType>
{
public:

	typedef void (RecordType::*RefRecordSetter)(const AutoPtr<RefRecordType>&);
    typedef void (RecordType::*PositionSetter)(const I32u&);
    typedef const I32u& (RefRecordType::*NestedCountGetter)() const;

	ClusteredReferenceHydrator(RefRecordSetter parentSetter, NestedCountGetter countGetter, RandomSetInspector<RefRecordType> parentSet, I64u nestedSetCardinality) :
		_parentSetter(parentSetter),
		_positionSetter(NULL),
		_countGetter(countGetter),
		_parentSet(parentSet),
		_referenceSetCardinality(parentSet.cardinality()),
        _nestedSetCardinality(nestedSetCardinality),
        _maxNestedPerParent(_nestedSetCardinality/_referenceSetCardinality),
        _parent(NULL)
	{
	}

	ClusteredReferenceHydrator(RefRecordSetter parentSetter, PositionSetter positionSetter, NestedCountGetter countGetter, RandomSetInspector<RefRecordType> parentSet, I64u nestedSetCardinality) :
		_parentSetter(parentSetter),
		_positionSetter(positionSetter),
		_countGetter(countGetter),
		_parentSet(parentSet),
		_referenceSetCardinality(parentSet.cardinality()),
        _nestedSetCardinality(nestedSetCardinality),
        _maxNestedPerParent(_nestedSetCardinality/_referenceSetCardinality),
        _parent(NULL)
	{
	}

	void operator()(AutoPtr<RecordType> recordPtr) const
	{
		if (RecordHydrator<RecordType>::_enabled)
		{
		    I64u nestedRecordGenID = recordPtr->genID();
		    I64u parentRecordGenID = nestedRecordGenID/_maxNestedPerParent;
		    
		    if (_parent.isNull() || _parent->genID() != parentRecordGenID)
		    {
		        const_cast< ClusteredReferenceHydrator<RecordType, RefRecordType>* >(this)->_parent = _parentSet.at(parentRecordGenID);
		    }
		    
		    I64u nestedCount = (_parent->*_countGetter)();

		    if (nestedRecordGenID % _maxNestedPerParent < nestedCount)
		    {
		        (recordPtr->*_parentSetter)(_parent);

		        if (_positionSetter != NULL)
				{
		        	(recordPtr->*_positionSetter)(static_cast<I32u>(nestedRecordGenID-(parentRecordGenID*_maxNestedPerParent)));
				}
		    }
		    else
		    {
		        throw InvalidRecordException(nestedRecordGenID, (parentRecordGenID+1)*_maxNestedPerParent);
		    }
		}
	}

private:

	RefRecordSetter _parentSetter;

	PositionSetter _positionSetter;

	NestedCountGetter _countGetter;

	RandomSetInspector<RefRecordType> _parentSet;

	I64u _referenceSetCardinality;

    I64u _nestedSetCardinality;

    I32u _maxNestedPerParent;

    AutoPtr<RefRecordType> _parent;
};

}  // namespace Myriad


#endif /* CLUSTEREDREFERENCEHYDRATOR_H_ */
