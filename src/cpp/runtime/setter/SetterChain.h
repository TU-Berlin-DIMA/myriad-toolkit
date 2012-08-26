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

#ifndef SETTERCHAIN_H_
#define SETTERCHAIN_H_

#include "runtime/predicate/EqualityPredicate.h"

#include <Poco/AutoReleasePool.h>
#include <Poco/AutoPtr.h>
#include <vector>

using namespace Poco;
using namespace std;

namespace Myriad {

class BaseSetterChain
{
public:

    enum OperationMode
    {
        SEQUENTIAL, RANDOM
    };

    BaseSetterChain(OperationMode opMode, RandomStream& random) : _random(random), _opMode(opMode)
    {
    }

protected:

    void ensurePosition(I64u position) const;

    RandomStream& _random;

private:

    const OperationMode _opMode;
};

inline void BaseSetterChain::ensurePosition(I64u position) const
{
    if (_opMode == RANDOM)
    {
        const_cast<BaseSetterChain*>(this)->_random.atChunk(position);
    }
}

/**
 * Interface template for all SetterChain.
 */
template<class RecordType>
class SetterChain : public BaseSetterChain
{
public:

    SetterChain<RecordType>(OperationMode opMode, RandomStream& random) : BaseSetterChain(opMode, random)
    {
    }

    virtual ~SetterChain<RecordType>()
    {
    }

    /**
     * Applies the setter chain to the given record instance.
     */
    virtual void operator()(AutoPtr<RecordType> recordPtr) const = 0;

    /**
     * Predicate filter function.
     */
    virtual Interval<I64u> filter(const EqualityPredicate<RecordType>& predicate) = 0;
};

} // namespace Myriad

#endif /* SETTERCHAIN_H_ */
