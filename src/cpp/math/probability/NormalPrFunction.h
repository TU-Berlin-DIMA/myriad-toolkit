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

#ifndef NORMALPRFUNCTION_H_
#define NORMALPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"

#include <Poco/Any.h>

#include <string>
#include <map>
#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup math_probability
 * @{*/

/**
 * A normal probability function implementation.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class NormalPrFunction: public UnivariatePrFunction<Decimal>
{
public:

    /**
     * Explicit anonymous parameter constructor.
     *
     * @param mean The \p mean of this distribution.
     * @param stddev The <tt>standard deviation</tt> of this distribution.
     */
    NormalPrFunction(Decimal mean = 0, Decimal stddev = 1) :
        UnivariatePrFunction<Decimal> (""), _mean(mean), _stddev(stddev)
    {
        initialize();
    }

    /**
     * Explicit named parameter constructor.
     *
     * @param name The name of this probability function instance.
     * @param mean The \p mean of this distribution.
     * @param stddev The <tt>standard deviation</tt> of this distribution.
     */
    NormalPrFunction(const string& name, Decimal mean = 0, Decimal stddev = 1) :
        UnivariatePrFunction<Decimal> (name), _mean(mean), _stddev(stddev)
    {
        initialize();
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * @param params An array containing the required function parameters.
     */
    NormalPrFunction(map<string, Any>& params) :
        UnivariatePrFunction<Decimal> ("")
    {
        _mean = AnyCast<Decimal>(params["mean"]);
        _stddev = AnyCast<Decimal>(params["stddev"]);

        initialize();
    }

    /**
     * Named ObjectBuilder Constructor.
     *
     * @param name The name of this probability function instance.
     * @param params An array containing the required function parameters.
     */
    NormalPrFunction(const string& name, map<string, Any>& params) :
        UnivariatePrFunction<Decimal> (name)
    {
        _mean = AnyCast<Decimal>(params["mean"]);
        _stddev = AnyCast<Decimal>(params["stddev"]);

        initialize();
    }

    /**
     * @see UnivariatePrFunction::operator()
     */
    Decimal operator()(const Decimal x) const
    {
        return cdf(x);
    }

    /**
     * @see UnivariatePrFunction::sample()
     */
    Decimal sample(Decimal random) const
    {
        return invcdf(random);
    }

    /**
     * @see UnivariatePrFunction::pdf()
     */
    Decimal pdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::cdf()
     */
    Decimal cdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::invpdf()
     */
    Decimal invpdf(Decimal y) const;

    /**
     * @see UnivariatePrFunction::invcdf()
     */
    Decimal invcdf(Decimal y) const;

    /**
     * FIXME: this is suspicious.
     */
    Interval<Decimal> threshold(Decimal yMin) const;

    /**
     * Returns the normal probability function \p mean parameter.
     */
    Decimal mean() const;

    /**
     * Returns the normal probability function \p stddev parameter.
     */
    Decimal stddev() const;

private:

    /**
     * Common initialization logic.
     */
    void initialize()
    {
        _var = _stddev * _stddev;
        _A = 1 / sqrt(2 * M_PI * _var);

        _a = 0.14;
    }

    Decimal erf(const Decimal x) const;

    Decimal inverf(const Decimal y) const;

    // parameters
    Decimal _mean;
    Decimal _stddev;

    // common used terms
    Decimal _var;
    Decimal _A;
    Decimal _a;
};

/** @}*/// add to math group
} // namespace Myriad

#endif /* NORMALPRFUNCTION_H_ */
