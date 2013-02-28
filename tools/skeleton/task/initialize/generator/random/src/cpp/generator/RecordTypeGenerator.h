/*
 * Copyright 2010-2013 DIMA Research Group, TU Berlin
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

#include "generator/RandomSequenceGenerator.h"
#include "runtime/setter/${{record_name}}SetterChain.h"

namespace ${{dgen_ns}} {

class ${{record_name}}Generator: public RandomSequenceGenerator<${{record_name}}>
{
public:

    typedef Myriad::RecordTraits<${{record_name}}>::SetterChainType SetterChainType;

    ${{record_name}}Generator(const string& name, Myriad::GeneratorConfig& config, NotificationCenter& notificationCenter) :
        RandomSequenceGenerator<${{record_name}}>(name, config, notificationCenter)
    {
    }

    void prepare(Stage stage, const GeneratorPool& pool)
    {
        // call generator implementation
        RandomSequenceGenerator<${{record_name}}>::prepare(stage, pool);

        if (stage.name() == name())
        {
	        registerTask(new PartitionedSequenceIteratorTask<${{record_name}}> (*this, _config));
        }
    }

    ${{record_name}}SetterChain setterChain(BaseSetterChain::OperationMode opMode, RandomStream& random)
    {
        return ${{record_name}}SetterChain(opMode, random, _config);
    }
};

} // namespace ${{dgen_ns}}

#endif /* ${uc{record_name}}GENERATOR_H_ */

