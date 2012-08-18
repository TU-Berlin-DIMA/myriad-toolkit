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

#ifndef ${uc{record_name}}SETTERCHAIN_H_
#define ${uc{record_name}}SETTERCHAIN_H_

#include "runtime/setter/SetterChain.h"
#include "record/${{record_name}}.h"
#include "record/${{record_name}}Util.h"

using namespace Myriad;

namespace ${{dgen_ns}} {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// SetterChain specialization (base class)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * SetterChain specialization for ${{record_name}}.
 */
class ${{record_name}}SetterChain : public SetterChain<${{record_name}}>
{
public:

	// runtime components type aliases

	${{record_name}}SetterChain(OperationMode& opMode, RandomStream& random, GeneratorConfig& config) :
		SetterChain<${{record_name}}>(opMode, random),
        _sequenceCardinality(config.cardinality("${cc2us{record_name}}")),
		_logger(Logger::get("${cc2us{record_name}}.setter.chain"))
	{
	}

	virtual ~${{record_name}}SetterChain()
	{
	}

	/**
	 * Applies the setter chain to the given record instance.
	 */
	void operator()(AutoPtr<${{record_name}}> recordPtr) const
	{
		ensurePosition(recordPtr->genID());

		// apply setter chain
	}

    /**
     * Predicate filter function.
     */
    virtual Interval<I64u> filter(const EqualityPredicate<${{record_name}}>& predicate)
    {
        Interval<I64u> result(0, _sequenceCardinality);

        // apply inverse setter chain

        return result;
    }

protected:

    // cardinality
    I64u _sequenceCardinality;

	// runtime components

    // Logger instance
	Logger& _logger;
};

} // namespace ${{dgen_ns}}

#endif /* ${uc{record_name}}SETTERCHAIN_H_ */

