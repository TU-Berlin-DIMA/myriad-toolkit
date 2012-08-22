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
/**
 * @addtogroup math
 * @{*/

/**
 * An abstract base class for all functions.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class AbstractFunction : public Poco::RefCountedObject
{
public:

    /**
     * Named constructor.
     *
     * @param name The name of this function.
     */
	AbstractFunction(const string& name) :
		_name(name)
	{
	}

	/**
	 * Return the name of the function.
	 *
	 * @return The name of the function.
	 */
	const string& name() const
	{
	    return _name;
	}

protected:

	/**
	 * Protected destructor (prohibit static allocation).
	 */
	virtual ~AbstractFunction()
	{
	}

	const string _name;
};

/**
 * A an abstract template base for all unary functions.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class Domain, class Range>
class UnaryFunction: public std::unary_function<Domain, Range>, public AbstractFunction
{
public:

    /**
     * Named constructor.
     *
     * @param name The name of this function.
     */
	UnaryFunction(const string& name) :
		AbstractFunction(name)
	{
	}

	/**
	 * Function evaluation operator.
	 *
	 * @param x The function argument.
	 * @return The <tt>f(x)</tt> value.
	 */
	virtual Range operator()(const Domain x) const = 0;

protected:

    /**
     * Protected destructor (prohibit static allocation).
     */
	virtual ~UnaryFunction()
	{
	}
};

/**
 * A an abstract template base for all binary functions.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class Domain1, class Domain2, class Range>
class BinaryFunction: public std::binary_function<Domain1, Domain2, Range>, public AbstractFunction
{
public:

    /**
     * Named constructor.
     *
     * @param name The name of this function.
     */
	BinaryFunction(const string& name) :
		AbstractFunction(name)
	{
	}

	/**
	 * Function evaluation operator.
     *
     * @param x1 The first function argument.
     * @param x1 The second function argument.
     * @return The <tt>f(x1, x2)</tt> value.
	 */
	virtual Range operator()(const Domain1 x1, const Domain2 x2) const = 0;

protected:

    /**
     * Protected destructor (prohibit static allocation).
     */
	virtual ~BinaryFunction()
	{
	}
};

/**
 * A base template for all univariate probability functions.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class Domain> class
UnivariatePrFunction: public UnaryFunction<Domain, Decimal>
{
public:

    /**
     * Named constructor.
     *
     * @param name The name of this function.
     */
	UnivariatePrFunction(const string& name) :
		UnaryFunction<Domain, Decimal>(name)
	{
	}

    /**
     * Alias for the UnivariatePrFunction::cdf() method.
     *
     * @return P(X <= x)
     */
	virtual Decimal operator()(const Domain x) const = 0;

	/**
	 * Returns the probability distribution function (PDF) for this
	 * distribution.
	 *
	 * @return P(X = x)
	 */
	virtual Decimal pdf(Domain x) const = 0;

	/**
	 * Returns the cumulative distribution function (CDF) for this distribution.
     *
     * @return P(X <= x)
	 */
	virtual Decimal cdf(Domain x) const = 0;

	/**
	 * Returns the inverse cumulative distribution function (pdf) for this
	 * distribution.
	 *
	 * @return A value for x, such that y is equal to P(X <= x).
	 */
	virtual Domain invcdf(Decimal y) const = 0;

	/**
	 * Performs an inverse transform sampling.
	 *
	 * If the \p r values are uniformly distributed, subsequent invocations of
	 * this method will yield a population \p Domain samples distributed
	 * according to this distribution function.
	 *
	 * @param r A uniformly drawn random value in the [0,1) interval.
	 * @param A \Domain sample that corresponds to the given input \p r.
	 */
	virtual Domain sample(Decimal r) const = 0;

protected:

    /**
     * Protected destructor (prohibit static allocation).
     */
	virtual ~UnivariatePrFunction()
	{
	}
};

/**
 * A base template for all conditional bivariate probability functions.
 */
template<class Domain1, class Domain2>
class BivariatePrFunction: public BinaryFunction<Domain1, Domain2, Decimal>
{
public:

    /**
     * Named constructor.
     *
     * @param name The name of this function.
     */
	BivariatePrFunction(const string& name) :
		BinaryFunction<Domain1, Domain2, Decimal>(name)
	{
	}

    /**
     * Alias for the UnivariatePrFunction::pdf() method.
     *
     * @return P(X1 = x1 | X2 = x2)
     */
	virtual Decimal operator()(const Domain1 x1, const Domain2 x2) const = 0;

    /**
     * Returns the probability distribution function (pdf) for this
     * distribution.
     *
     * @return P(X1 = x1 | X2 = x2)
     */
	virtual Decimal pdf(Domain1 x1, Domain2 x2) const = 0;

    /**
     * Returns the cumulative distribution function (cdf) for this
     * distribution.
     *
     * @return P(X1 <= x1 | X2 = x2)
     */
	virtual Decimal cdf(Domain1 x1, Domain2 x2) const = 0;

	/**
     * @return A value for x1, such that y is equal to P(X1 <= x1 | X2 = x2).
	 */
	virtual Domain1 invcdf(Decimal y, Domain2 x2) const = 0;

    /**
     * Performs a conditional inverse transform sampling.
     *
     * If the \p r values are uniformly distributed, subsequent invocations of
     * this method will yield a population \p Domain1 samples distributed
     * according to this distribution function.
     *
     * @param r A uniformly drawn random value in the [0,1) interval.
     * @param A \Domain1 sample that corresponds to the given input \p r.
     */
	virtual Domain1 sample(Decimal r, Domain2 x2) const = 0;

protected:

    /**
     * Protected destructor (prohibit static allocation).
     */
	virtual ~BivariatePrFunction()
	{
	}
};

/** @}*/// add to math group
} // namespace Myriad


#endif /* FUNCTION_H_ */
