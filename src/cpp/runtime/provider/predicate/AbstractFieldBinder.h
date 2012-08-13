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

#ifndef ABSTRACTFIELDBINDER_H_
#define ABSTRACTFIELDBINDER_H_

using namespace Poco;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// abstract field value binder for equality predicates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<class RecordType, class CxtRecordType>
class AbstractFieldBinder
{
public:

    typedef EqualityPredicate<RecordType> EqualityPredicateType;

    AbstractFieldBinder(const I16u arity) :
        _arity(arity)
    {
    }

    virtual ~AbstractFieldBinder()
    {
    }

    I16u arity() const
    {
        return _arity;
    }

    virtual void operator()(EqualityPredicateType& predicate, AutoPtr<CxtRecordType>& cxtRecordPtr, RandomStream& random) = 0;

private:

    const I16u _arity;
};

}  // namespace Myriad


#endif /* ABSTRACTFIELDBINDER_H_ */
