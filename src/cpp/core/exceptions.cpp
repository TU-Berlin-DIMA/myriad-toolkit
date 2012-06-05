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

#include "core/types.h"
#include "core/exceptions.h"

#include <typeinfo>

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// exception definitions
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

POCO_IMPLEMENT_EXCEPTION(ConfigException, Poco::Exception, "Error in configuration")
POCO_IMPLEMENT_EXCEPTION(FeatureConfigurationException, Poco::Exception, "Error while configuring feature")
POCO_IMPLEMENT_EXCEPTION(IntegerOverflowException, Poco::LogicException, "Integer overflow")

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// type constants
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~


const I16  NullValue::SHORT = std::numeric_limits<I16>::max();
const I32  NullValue::INTEGER = std::numeric_limits<I32>::max();;
const I64  NullValue::BIGINTEGER = std::numeric_limits<I64>::max();;
const I16u NullValue::USHORT = std::numeric_limits<I16u>::max();;
const I32u NullValue::UINTEGER = std::numeric_limits<I32u>::max();;
const I64u NullValue::UBIGINTEGER = std::numeric_limits<I64u>::max();;
const Decimal NullValue::DECIMAL = std::numeric_limits<Decimal>::max();;
const Date NullValue::DATE = Date(DateTime(9999, 12, 31));
const String NullValue::STRING = "NULL";

} // namespace Myriad
