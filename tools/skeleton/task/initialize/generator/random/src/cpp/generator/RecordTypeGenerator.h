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

#ifndef ${uc{record_name}}GENERATOR_H_
#define ${uc{record_name}}GENERATOR_H_

#include "generator/RandomSetGenerator.h"
#include "record/${{record_name}}.h"

using namespace Myriad;
using namespace Poco;

namespace ${{dgen_ns}} {

class ${{record_name}}SetterChain;

class ${{record_name}}Generator: public RandomSetGenerator<${{record_name}}>
{
public:

	typedef RecordTraits<${{record_name}}>::SetterChainType SetterChainType;

	${{record_name}}Generator(const string& name, GeneratorConfig& config, NotificationCenter& notificationCenter) :
		RandomSetGenerator<${{record_name}}>(name, config, notificationCenter)
	{
	}

	void prepare(Stage stage, const GeneratorPool& pool)
	{
		// call generator implementation
		RandomSetGenerator<${{record_name}}>::prepare(stage, pool);

		if (stage.name() == "default")
		{
			registerTask(new RandomSetDefaultGeneratingTask<${{record_name}}> (*this, _config));
		}
	}

	SetterChainType setterChain(BaseSetterChain::OperationMode opMode, RandomStream& random);
};

/**
 * SetterChain specialization for ${{record_name}}.
 */
class ${{record_name}}SetterChain : public SetterChain<${{record_name}}>
{
public:

	// runtime components type aliases

	${{record_name}}SetterChain(OperationMode& opMode, RandomStream& random, GeneratorConfig& config) :
		SetterChain<${{record_name}}>(opMode, random),
		_logger(Logger::get("${cc2us{record_name}}.setter.chain"))
	{
	}

	virtual ~${{record_name}}SetterChain()
	{
	}

	/**
	 * Object hydrating function.
	 */
	void operator()(AutoPtr<${{record_name}}> recordPtr) const
	{
		ensurePosition(recordPtr->genID());

		// apply setter chain
	}

protected:

	// runtime components

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// base method definitions (don't modify)
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

inline ${{record_name}}SetterChain ${{record_name}}Generator::setterChain(BaseSetterChain::OperationMode opMode, RandomStream& random)
{
	return ${{record_name}}SetterChain(opMode, random, _config);
}

} // namespace ${{dgen_ns}}

#endif /* ${uc{record_name}}GENERATOR_H_ */

