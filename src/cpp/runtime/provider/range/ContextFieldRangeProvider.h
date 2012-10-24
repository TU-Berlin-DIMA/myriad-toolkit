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
 */

#ifndef CONTEXTFIELDRANGEPROVIDER_H_
#define CONTEXTFIELDRANGEPROVIDER_H_

#include "runtime/provider/range/AbstractRangeProvider.h"

using namespace Poco;

namespace Myriad {
/**
 * @addtogroup runtime_provider_range
 * @{*/

/**
 * Range provider for context-based ranges.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename RangeType, class CxtRecordType, class InvertibleFieldSetterType>
class ContextFieldRangeProvider: public AbstractRangeProvider<RangeType, CxtRecordType>
{
public:

    /**
     * Constructor.
     *
     * @param fieldSetter An invertible field setter to get the range fo values
     *        from a \p CxtRecordType.
     */
    ContextFieldRangeProvider(InvertibleFieldSetterType& fieldSetter) :
        AbstractRangeProvider<RangeType, CxtRecordType>(0),
        _fieldSetter(fieldSetter)
    {
        if (!_fieldSetter.invertible())
        {
            throw LogicException("Cannot use non-invertible FieldSetter in ContextFieldRangeProvider");
        }
    }

    /**
     * Destructor.
     */
    virtual ~ContextFieldRangeProvider()
    {
    }

    /**
     * Functor method. Provides an interval of genID for which the corresponding
     * records in the \p CxtRecordType sequence have same value as the one
     * defined by the \p cxtRecordPtr parameter.
     *
     * @param cxtRecordPtr A context record for this range provider.
     */
    virtual const Interval<RangeType> operator()(const AutoPtr<CxtRecordType>& cxtRecordPtr)
    {
        return _fieldSetter.valueRange(cxtRecordPtr);
    }

private:

    InvertibleFieldSetterType& _fieldSetter;
};

/** @}*/// add to runtime_provider_range group
} // namespace Myriad

#endif /* CONTEXTFIELDRANGEPROVIDER_H_ */
