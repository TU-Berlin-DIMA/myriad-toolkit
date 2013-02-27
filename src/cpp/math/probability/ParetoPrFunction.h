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
 */

#ifndef PARETOPRFUNCTION_H_
#define PARETOPRFUNCTION_H_

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
 * A Pareto probability function implementation.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class ParetoPrFunction: public UnivariatePrFunction<Decimal>
{
public:

    /**
     * Explicit anonymous parameter constructor.
     *
     * @param xMin The scale (i.e. the left bound) of this distribution.
     * @param alpha The shape of this distribution.
     */
    ParetoPrFunction(Decimal xMin = 1, Decimal alpha = 1) :
        UnivariatePrFunction<Decimal> (""), xMin(xMin), alpha(alpha), xMinAlpha(pow(xMin, alpha))
    {
    }

    /**
     * Explicit named parameter constructor.
     *
     * @param name The name of this probability function instance.
     * @param xMin The scale (i.e. the left bound) of this distribution.
     * @param alpha The shape of this distribution.
     */
    ParetoPrFunction(const string& name, Decimal xMin = 1, Decimal alpha = 1) :
        UnivariatePrFunction<Decimal> (name), xMin(xMin), alpha(alpha), xMinAlpha(pow(xMin, alpha))
    {
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * @param params An array containing the required function parameters.
     */
    ParetoPrFunction(map<string, Any>& params) :
        UnivariatePrFunction<Decimal> ("")
    {
        xMin = AnyCast<Decimal>(params["xMin"]);
        alpha = AnyCast<Decimal>(params["alpha"]);
        xMinAlpha = pow(xMin, alpha);
    }

    /**
     * Named ObjectBuilder Constructor.
     *
     * @param name The name of this probability function instance.
     * @param params An array containing the required function parameters.
     */
    ParetoPrFunction(const string& name, map<string, Any>& params) :
        UnivariatePrFunction<Decimal> (name)
    {
        xMin = AnyCast<Decimal>(params["xMin"]);
        alpha = AnyCast<Decimal>(params["alpha"]);
        xMinAlpha = pow(xMin, alpha);
    }

    /**
     * @see UnivariatePrFunction::operator()
     */
    Decimal operator()(const Decimal x) const;


    /**
     * @see UnivariatePrFunction::pdf()
     */
    Decimal pdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::cdf()
     */
    Decimal cdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::invcdf()
     */
    Decimal invcdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::invpdf()
     */
    Decimal invpdf(Decimal x) const;

    /**
     * @see UnivariatePrFunction::sample()
     */
    Decimal sample(Decimal random) const;

private:

    // parameters
    Decimal xMin;
    Decimal alpha;

    // common used terms
    Decimal xMinAlpha;
};

inline Decimal ParetoPrFunction::operator()(const Decimal x) const
{
    return cdf(x);
}

inline Decimal ParetoPrFunction::sample(Decimal random) const
{
    return invcdf(random);
}

/** @}*/// add to math_probability group
} // namespace Myriad


#endif /* PARETOPRFUNCTION_H_ */
