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

#ifndef CONDITIONALCOMBINEDPRFUNCTION_H_
#define CONDITIONALCOMBINEDPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/probability/CombinedPrFunction.h"

#include <Poco/Any.h>
#include <Poco/Exception.h>
#include <Poco/String.h>
#include <Poco/RegularExpression.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{
/**
 * @addtogroup math_probability
 * @{*/

/**
 * A custom conditional probability function.
 *
 * Conceptually, the implementation is a wrapper around a collection of
 * CombinedProbabilityFunction objects, each one defining the probability
 * conditioned on a range of \p T2 values from the condition domain.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<typename T1, typename T2>
class ConditionalCombinedPrFunction: public BivariatePrFunction<T1, T2>
{
public:

    /**
     * Anonymous file path initialization constructor.
     *
     * Loads the configuration for this probability function from the file
     * given by the \p path parameter.
     *
     * @param path The location of the function configuration file.
     */
    ConditionalCombinedPrFunction(const string& path) :
        BivariatePrFunction<T1, T2>(""), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(path);
    }

    /**
     * Named file path initialization constructor.
     *
     * Loads the configuration for this probability function from the file
     * given by the \p path parameter.
     *
     * @param name The name of this probability function instance.
     * @param path The location of the function configuration file.
     */
    ConditionalCombinedPrFunction(const string& name, const string& path) :
        BivariatePrFunction<T1, T2>(name), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(path);
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * Loads the configuration for this probability function from the input
     * stream given by the <tt>params['path']</tt> in parameter.
     *
     * @param params An array containing the required function parameters.
     */
    ConditionalCombinedPrFunction(map<string, Any>& params) :
        BivariatePrFunction<T1, T2>(""), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(AnyCast<string>(params["path"]));
    }

    /**
     * Named ObjectBuilder constructor.
     *
     * Loads the configuration for this probability function from the input
     * stream given by the <tt>params['path']</tt> in parameter.
     *
     * @param name The name of this probability function instance.
     * @param params An array containing the required function parameters.
     */
    ConditionalCombinedPrFunction(const string& name, map<string, Any>& params) :
        BivariatePrFunction<T1, T2>(name), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(AnyCast<string>(params["path"]));
    }

    /**
     * Destructor.
     */
    virtual ~ConditionalCombinedPrFunction()
    {
        reset();
    }

    /**
     * Initialization routine.
     *
     * Initializes the function with the configuration stored in the file
     * located at the given \p path.
     *
     * @param path The location of the function configuration file.
     */
    void initialize(const string& path);

    /**
     * Initialization routine.
     *
     * Initializes the function with the configuration from the input stream
     * given by the \p in parameter.
     *
     * @param in Input stream containing the function configuration.
     */
    void initialize(istream& path);

    /**
     * @see UnivariatePrFunction::operator()
     */
    Decimal operator()(const T1 x1, const T2 x2) const;

    /**
     * @see UnivariatePrFunction::pdf()
     */
    Decimal pdf(T1 x1, T2 x2) const;

    /**
     * @see UnivariatePrFunction::cdf()
     */
    Decimal cdf(T1 x1, T2 x2) const;

    /**
     * @see UnivariatePrFunction::invcdf()
     */
    T1 invcdf(Decimal y, T2 x2) const;

    /**
     * @see UnivariatePrFunction::sample()
     */
    T1 sample(Decimal r, T2 x2) const;

private:

    void reset();

    size_t findBucket(const T2 x) const;

    static RegularExpression headerLine1Format;
    static RegularExpression headerLine2Format;

    size_t _numberOfx2Buckets;
    Interval<T2>* _x2Buckets;
    CombinedPrFunction<T1>* _x1Pr;
};

////////////////////////////////////////////////////////////////////////////////
/// @name Static Template Members
////////////////////////////////////////////////////////////////////////////////
//@{

template<typename T1, typename T2> RegularExpression ConditionalCombinedPrFunction<T1, T2>::headerLine1Format("\\W*@numberofconditions\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T1, typename T2> RegularExpression ConditionalCombinedPrFunction<T1, T2>::headerLine2Format("\\W*@condition\\W*=\\W*\\[\\W*(.+)\\W*,\\W*(.+)\\W*\\)\\W*(#(.+))?");

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Member Functions
////////////////////////////////////////////////////////////////////////////////
//@{

template<typename T1, typename T2> inline void ConditionalCombinedPrFunction<T1, T2>::reset()
{
    if (_numberOfx2Buckets > 0)
    {
        _numberOfx2Buckets = 0;
        delete[] _x2Buckets;
        delete[] _x1Pr;
    }
}

template<typename T1, typename T2> inline size_t ConditionalCombinedPrFunction<T1, T2>::findBucket(const T2 x) const
{
    // we assert that the value x is in the [_min, _max] range
    int min = 0;
    int max = _numberOfx2Buckets - 1;
    int mid = 0;

    // continue searching while [min, max] is not empty
    while (max >= min)
    {
        // calculate the midpoint for roughly equal partition //
        mid = (min + max) / 2;

        // determine which subarray to search
        if (_x2Buckets[mid].max() <=  x)
        {
            // change min index to search upper subarray
            min = mid + 1;
        }
        else if (_x2Buckets[mid].min() > x)
        {
            // change max index to search lower subarray
            max = mid - 1;
        }
        else
        {
            // key found at index mid
            return mid;
        }
    }

    return nullValue<size_t>();
}

template<typename T1, typename T2> inline Decimal ConditionalCombinedPrFunction<T1, T2>::operator()(const T1 x1, const T2 x2) const
{
    return cdf(x1, x2);
}

template<typename T1, typename T2> inline T1 ConditionalCombinedPrFunction<T1, T2>::sample(Decimal r, T2 x2) const
{
    return invcdf(r, x2);
}

/**
 * Load a Q-histogram from the given path.
 */
template<typename T1, typename T2>
void ConditionalCombinedPrFunction<T1, T2>::initialize(const string& path)
{
    ifstream in(path.c_str());

    if (!in.is_open())
    {
        throw OpenFileException("Unexpected file header for file `" + path +  "`");
    }

    try
    {
        initialize(in);

        in.close();
    }
    catch(Poco::Exception& e)
    {
        in.close();
        throw e;
    }
    catch(std::exception& e)
    {
        in.close();
        throw e;
    }
    catch(...)
    {
        in.close();
        throw;
    }
}

/**
 * Load the distribution data from the given input stream. For each entry add a
 * [min, max) interval of domain values to a lookup table.
 */
template<typename T1, typename T2>
void ConditionalCombinedPrFunction<T1, T2>::initialize(istream& in)
{
    enum READ_STATE { NOC, CON, END };

    // reset old state
    reset();

    // reader variables
    READ_STATE currentState = NOC; // current reader machine state
    string currentLine; // the current line
    I16u currentX2BucketIndex = 0; // current item index
    I16u currentLineNumber = 1; // current line number
    RegularExpression::MatchVec posVec; // a posVec for all regex matches

    // reader finite state machine
    while (currentState != END)
    {
        // read next line
        getline(in, currentLine);

        // trim whitespace
        trimInPlace(currentLine);

        // check if this line is empty or contains a single comment
        if (currentLine.empty() || currentLine.at(0) == '#')
        {
	        currentLineNumber++;
	        continue; // skip this line
        }

        if (currentState == NOC)
        {
	        if (!in.good() || !headerLine1Format.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofconditions = [' + x", currentLineNumber, currentLine));
	        }

	        I32 numberOfx2Buckets = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

	        if (numberOfx2Buckets <= 0 && numberOfx2Buckets > 65536)
	        {
	            throw DataException("Invalid number of conditions`" + toString(numberOfx2Buckets) +  "`");
	        }

	        _numberOfx2Buckets = numberOfx2Buckets;
	        _x2Buckets = new Interval<T2>[numberOfx2Buckets];
	        _x1Pr = new CombinedPrFunction<T1>[numberOfx2Buckets];

	        currentX2BucketIndex = 0;
	        currentState = (numberOfx2Buckets > 0) ? CON : END;
        }
        else if (currentState == CON)
        {
	        if (!in.good() || !headerLine2Format.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@condition = [' + x + ', ' + y + ')'", currentLineNumber, currentLine));
	        }

            T2 min = fromString<T2>(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());
            T2 max = fromString<T2>(currentLine.substr(posVec[2].offset, posVec[2].length).c_str());

            _x2Buckets[currentX2BucketIndex].set(min, max);
            _x1Pr[currentX2BucketIndex].initialize(in, currentLineNumber);

	        currentX2BucketIndex++;

	        if (currentX2BucketIndex >= _numberOfx2Buckets)
	        {
		        currentState = END;
		        currentX2BucketIndex = 0;
	        }
        }

        currentLineNumber++;
    }

    // protect against unexpected reader state
    if (currentState != END)
    {
        throw RuntimeException("Unexpected state in ConditionalCombinedPrFunction reader at line " + currentLineNumber);
    }
}

template<typename T1, typename T2>
Decimal ConditionalCombinedPrFunction<T1, T2>::pdf(T1 x1, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].pdf(x1);
}

template<typename T1, typename T2>
Decimal ConditionalCombinedPrFunction<T1, T2>::cdf(T1 x1, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].cdf(x1);
}

template<typename T1, typename T2>
T1 ConditionalCombinedPrFunction<T1, T2>::invcdf(Decimal y, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].invcdf(y);
}

//@}

/** @}*/// add to math group
} // namespace Myriad

#endif /* CONDITIONALCOMBINEDPRFUNCTION_H_ */
