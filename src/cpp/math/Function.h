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
 * A base template for all binary functions.
 */
template<class Domain1, class Domain2, class Range> class BinaryFunction: public std::binary_function<Domain1, Domain2, Range>, public AbstractFunction
{
public:
	BinaryFunction(const string& name) :
		AbstractFunction(name)
	{
	}

	/**
	 * Function evaluation operator.
	 *
	 * @param x1
	 * @param x2
	 * @return
	 */
	virtual Range operator()(const Domain1 x1, const Domain2 x2) const = 0;

protected:

	virtual ~BinaryFunction()
	{
	}
};

/**
 * A base template for all univariate probability functions.
 */
template<class Domain> class UnivariatePrFunction: public UnaryFunction<Domain, Decimal>
{
public:
	UnivariatePrFunction(const string& name) :
		UnaryFunction<Domain, Decimal>(name)
	{
	}

	virtual Decimal operator()(const Domain x) const = 0;

	/**
	 * Returns the probability distribution function (pdf) for this
	 * distribution, i.e. Pr[X = x].
	 */
	virtual Decimal pdf(Domain x) const = 0;

	/**
	 * Returns the cumulative distribution function (pdf) for this
	 * distribution, i.e. Pr[X \leq x].
	 */
	virtual Decimal cdf(Domain x) const = 0;

	/**
	 * Returns the inverse cumulative distribution function (pdf) for this
	 * distribution, i.e. x such that Pr[X \leq x] = y.
	 */
	virtual Domain invcdf(Decimal y) const = 0;

	/**
	 * Transforms a uniform sample r sample from the underlying distribution
	 */
	virtual Domain sample(Decimal r) const = 0;

protected:

	virtual ~UnivariatePrFunction()
	{
	}
};

/**
 * A base template for all bivariate probability functions.
 */
template<class Domain1, class Domain2> class BivariatePrFunction: public BinaryFunction<Domain1, Domain2, Decimal>
{
public:
	BivariatePrFunction(const string& name) :
		BinaryFunction<Domain1, Domain2, Decimal>(name)
	{
	}

	virtual Decimal operator()(const Domain1 x1, const Domain2 x2) const = 0;

	virtual Decimal pdf(Domain1 x1, Domain2 x2) const = 0;

	virtual Decimal cdf(Domain1 x1, Domain2 x2) const = 0;

	virtual Domain1 invcdf(Decimal y, Domain2 x2) const = 0;

	virtual Domain1 sample(Decimal r, Domain2 x2) const = 0;

protected:

	virtual ~BivariatePrFunction()
	{
	}
};

} // namespace Myriad


#endif /* FUNCTION_H_ */
