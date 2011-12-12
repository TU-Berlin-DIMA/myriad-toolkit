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

#include "config/FunctionPool.h"

namespace Myriad {

void FunctionPool::addFunction(AbstractFunction* functionPtr)
{
	autoreleasePool.add(functionPtr);
	functions[functionPtr->name()] = functionPtr;
}

AbstractFunction* FunctionPool::function(const string& name)
{
	map<string, AbstractFunction*>::iterator it = functions.find(name);
	if (it != functions.end())
	{
		return it->second;
	}
	else
	{
		throw ConfigException("Trying to access undefined function " + name);
	}
}

} // namespace Myriad
