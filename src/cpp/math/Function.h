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

#ifndef FUNCTION_H_
#define FUNCTION_H_

#include "core/types.h"
#include "math/Interval.h"

#include <Poco/DynamicAny.h>
#include <Poco/RefCountedObject.h>

#include <functional>
#include <map>
#include <string>

using std::string;

namespace Myriad {

class AbstractFunction : public Poco::RefCountedObject
{
public:

	AbstractFunction(const string& name) :
		_name(name)
	{
	}

	/**
	 * Returns the name of the function
	 * @return
	 */
	const string& name() const;

protected:

	virtual ~AbstractFunction()
	{
	}

	const string _name;
};

inline const string& AbstractFunction::name() const
{
	return _name;
}

/**
 * A base template for all unary functions.
 */
template<class Domain, class Range> class UnaryFunction: public std::unary_function<Domain, Range>, public AbstractFunction
{
public:
	UnaryFunction(const string& name) :
		AbstractFunction(name)
	{
	}

	/**
	 * Function evaluation operator.
	 *
	 * @param x
	 * @return
	 */
	virtual Range operator()(const Domain x) const = 0;

protected:

	virtual ~UnaryFunction()
	{
	}
};

/**
 * A base template for all analytic probability distribution functions.
 */
template<class Domain> class AnalyticPrFunction: public UnaryFunction<Domain, Decimal>
{
public:
	AnalyticPrFunction(const string& name) :
		UnaryFunction<Domain, Decimal>(name)
	{
	}

	/**
	 * Function evaluation operator.
	 *
	 * @param x
	 * @return
	 */
	virtual Decimal operator()(const Domain x) const = 0;

	virtual Decimal pdf(Domain x) const = 0;

	virtual Decimal cdf(Domain x) const = 0;

	virtual Domain invcdf(Decimal y) const = 0;

	virtual Decimal sample(Decimal random) const = 0;

	virtual Interval<Domain> threshold(Decimal yMin) const = 0;

protected:

	virtual ~AnalyticPrFunction()
	{
	}
};

} // namespace Myriad


#endif /* FUNCTION_H_ */
