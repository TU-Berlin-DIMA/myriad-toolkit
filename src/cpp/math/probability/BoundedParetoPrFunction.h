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

#ifndef BOUNDEDPARETOPRFUNCTION_H_
#define BOUNDEDPARETOPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"

#include <cmath>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup math_probability
 * @{*/

/**
 * A bounded normal probability function variant (experimental).
 *
 * \b ATTENTION: Using this variant of the normal pr. function is not suitable,
 * if you the data generator program to produce outliers!!!
 *
 * @deprecated
 * @todo: This code needs to be revised.
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class BoundedParetoPrFunction: public UnivariatePrFunction<I64u>
{
public:

    /**
     * Explicit anonymous parameter constructor.
     *
     * @param alpha The \p alpha parameter of the Pareto distribution.
     * @param xMin The left bound of the function range.
     * @param xMax The right bound of the function range.
     */
    BoundedParetoPrFunction(Decimal xMin = 1, Decimal xMax = 100, Decimal alpha = 1) :
        UnivariatePrFunction<I64u> (""), xMin(xMin), xMax(xMax), alpha(alpha)
    {
        initialize();
    }

    /**
     * Explicit named parameter constructor.
     *
     * @param name The name of this probability function instance.
     * @param alpha The \p alpha parameter of the Pareto distribution.
     * @param xMin The left bound of the function range.
     * @param xMax The right bound of the function range.
     */
    BoundedParetoPrFunction(const string& name, Decimal xMin = 1, Decimal xMax = 100, Decimal alpha = 1) :
        UnivariatePrFunction<I64u> (name), xMin(xMin), xMax(xMax), alpha(alpha)
    {
        initialize();
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * @param params An array containing the required function parameters.
     */
    BoundedParetoPrFunction(map<string, DynamicAny> params) :
        UnivariatePrFunction<I64u> ("")
    {
        xMin = params["xMin"].convert<Decimal> ();
        xMax = params["xMax"].convert<Decimal> ();
        alpha = params["alpha"].convert<Decimal> ();

        initialize();
    }

    /**
     * Named ObjectBuilder Constructor.
     *
     * @param name The name of this probability function instance.
     * @param params An array containing the required function parameters.
     */
    BoundedParetoPrFunction(const string& name, map<string, DynamicAny> params) :
        UnivariatePrFunction<I64u> (name)
    {
        xMin = params["xMin"].convert<Decimal> ();
        xMax = params["xMax"].convert<Decimal> ();
        alpha = params["alpha"].convert<Decimal> ();

        initialize();
    }


    /**
     * \see UnivariatePrFunction::sample()
     */
    virtual Decimal pdf(I64u x) const;

    /**
     * \see UnivariatePrFunction::cdf()
     */
    virtual Decimal cdf(I64u x) const;

    /**
     * \see UnivariatePrFunction::invpdf()
     */
    I64u invpdf(Decimal x) const;

    /**
     * TODO: document
     */
    I64u invcdf(Decimal y) const;

private:

    void initialize()
    {
        xMinAlpha = pow(xMin, alpha);
        xMaxAlpha = pow(xMax, alpha);
        B = 1 - xMinAlpha/xMaxAlpha;
    }

    // parameters
    Decimal xMin;
    Decimal xMax;
    Decimal alpha;

    // common used terms
    Decimal xMinAlpha;
    Decimal xMaxAlpha;
    Decimal B;
};

/** @}*/// add to math_probability group
} // namespace Myriad


#endif /* BOUNDEDPARETTOPRFUNCTION_H_ */
