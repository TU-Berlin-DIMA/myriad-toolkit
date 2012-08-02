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

#ifndef CONDITIONALCOMBINEDPRFUNCTION_H_
#define CONDITIONALCOMBINEDPRFUNCTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/probability/CombinedPrFunction.h"

#include <Poco/Exception.h>
#include <Poco/Any.h>

#include <string>
#include <iostream>
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{

template<typename T1, typename T2> class ConditionalCombinedPrFunction: public BivariatePrFunction<T1, T2>
{
public:

    ConditionalCombinedPrFunction(const string& path) :
        BivariatePrFunction<T1, T2>(""), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(path);
    }

    ConditionalCombinedPrFunction(const string& name, const string& path) :
        BivariatePrFunction<T1, T2>(name), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(path);
    }

    ConditionalCombinedPrFunction(map<string, Any>& params) :
        BivariatePrFunction<T1, T2>(""), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(AnyCast<string>(params["path"]));
    }

    ConditionalCombinedPrFunction(const string& name, map<string, Any>& params) :
        BivariatePrFunction<T1, T2>(name), _numberOfx2Buckets(0), _x1Pr(NULL)
    {
        initialize(AnyCast<string>(params["path"]));
    }

    virtual ~ConditionalCombinedPrFunction()
    {
        reset();
    }

    void initialize(const string& path);

    void initialize(istream& path);

    Decimal operator()(const T1 x1, const T2 x2) const;

    Decimal pdf(T1 x1, T2 x2) const;

    Decimal cdf(T1 x1, T2 x2) const;

    T1 invcdf(Decimal y, T2 x2) const;

    T1 sample(Decimal r, T2 x2) const;

private:

    void reset();

    size_t findBucket(const T2 x) const;

    size_t _numberOfx2Buckets;
    Interval<T2>* _x2Buckets;
    CombinedPrFunction<T1>* _x1Pr;
};

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// private member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

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

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// inline member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

template<typename T1, typename T2> inline Decimal ConditionalCombinedPrFunction<T1, T2>::operator()(const T1 x1, const T2 x2) const
{
    return cdf(x1, x2);
}

template<typename T1, typename T2> inline T1 ConditionalCombinedPrFunction<T1, T2>::sample(Decimal r, T2 x2) const
{
    return invcdf(r, x2);
}

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// member function templates
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Load a Q-histogram from the given path.
 */
template<typename T1, typename T2>  void ConditionalCombinedPrFunction<T1, T2>::initialize(const string& path)
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
    catch(exception& e)
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
template<typename T1, typename T2> void ConditionalCombinedPrFunction<T1, T2>::initialize(istream& in)
{
    // reset old state
    reset();

    string line;

    // read first line
    getline(in, line);
    if (!in.good() || line.substr(0, 21) != "# numberofconditions:")
    {
        throw DataException("Unexpected file header (line 1)");
    }
    I32 numberOfx2Buckets = atoi(line.substr(21).c_str());

    if (numberOfx2Buckets <= 0 && numberOfx2Buckets > 65536)
    {
        throw DataException("Invalid number of conditions`" + toString(numberOfx2Buckets) +  "`");
    }

    _numberOfx2Buckets = numberOfx2Buckets;
    _x2Buckets = new Interval<T1>[numberOfx2Buckets];
    _x1Pr = new CombinedPrFunction<T1>[numberOfx2Buckets];

    // read second (empty) line
    getline(in, line);

    for (I16u i = 0; i < numberOfx2Buckets; i++)
    {
        if (!in.good())
        {
            throw DataException("Bad line for bin #" + toString(i));
        }

        getline(in, line);
        if (!in.good() || line.substr(0, 12) != "# condition:")
        {
            throw DataException("Unexpected condition format, expected format is `# condition: {min}{TAB}{max}`");
        }

        size_t tab1 = line.find_first_of('\t');
        if (tab1 == string::npos)
        {
            throw DataException("Unexpected condition format, expected format is `# condition: {min}{TAB}{max}`");
        }

        T2 min = fromString<T2>(line.substr(12, tab1-12));
        T2 max = fromString<T2>(line.substr(tab1+1));

        _x2Buckets[i].set(min, max);
        _x1Pr[i].initialize(in);
    }
}

template<typename T1, typename T2> Decimal ConditionalCombinedPrFunction<T1, T2>::pdf(T1 x1, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].pdf(x1);
}

template<typename T1, typename T2> Decimal ConditionalCombinedPrFunction<T1, T2>::cdf(T1 x1, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].cdf(x1);
}

template<typename T1, typename T2> T1 ConditionalCombinedPrFunction<T1, T2>::invcdf(Decimal y, T2 x2) const
{
    size_t i = findBucket(x2);

    if (i == nullValue<size_t>())
    {
        throw LogicException(format("Unknown distribution for evidence x2 = '%s'", toString<T2>(x2)));
    }

    return _x1Pr[i].invcdf(y);
}

} // namespace Myriad

#endif /* CONDITIONALCOMBINEDPRFUNCTION_H_ */
