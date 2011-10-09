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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#include "core/exceptions.h"

#include <typeinfo>

namespace Myriad {

POCO_IMPLEMENT_EXCEPTION(ConfigException, Poco::Exception, "Error in configuration")
POCO_IMPLEMENT_EXCEPTION(FeatureConfigurationException, Poco::Exception, "Error while configuring feature")
POCO_IMPLEMENT_EXCEPTION(IntegerOverflowException, Poco::LogicException, "Integer overflow")

} // namespace Myriad
