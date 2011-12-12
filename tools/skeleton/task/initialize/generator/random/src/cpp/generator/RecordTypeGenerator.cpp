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

class ${{record_name}}HydratorChain;

class ${{record_name}}Generator: public RandomSetGenerator<${{record_name}}>
{
public:

	typedef RecordTraits<${{record_name}}>::HydratorChainType HydratorChainType;

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

	HydratorChainType hydratorChain(BaseHydratorChain::OperationMode opMode, RandomStream& random);
};

/**
 * Hydrator specialization for ${{record_name}}.
 */
class ${{record_name}}HydratorChain : public HydratorChain<${{record_name}}>
{
public:

	// hydrator type aliases

	${{record_name}}HydratorChain(OperationMode& opMode, RandomStream& random, GeneratorConfig& config) :
		HydratorChain<${{record_name}}>(opMode, random),
		_logger(Logger::get("hydrator.${cc2us{record_name}}"))
	{
	}

	/**
	 * Object hydrating function.
	 */
	void operator()(AutoPtr<${{record_name}}> recordPtr) const
	{
		ensurePosition(recordPtr->genID());

		// apply hydrators
	}

protected:

	// hydrator instances

	/**
	 * Logger instance.
	 */
	Logger& _logger;
};

} // namespace ${{dgen_ns}}

#endif /* ${uc{record_name}}GENERATOR_H_ */

