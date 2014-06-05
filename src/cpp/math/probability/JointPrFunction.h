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

#ifndef JOINTPRFUNCTION_H_
#define JOINTPRFUNCTION_H_

#include "core/exceptions.h"
#include "core/types.h"
#include "math/Function.h"

#include <Poco/Any.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <Poco/RegularExpression.h>

#include <string>
#include <fstream>

using namespace std;
using namespace Poco;

namespace Myriad
{
/**
 * @addtogroup math_probability
 * @{*/

/**
 * A custom probability function consisting of a set of probabilities for
 * distinct values, a set of bucket probabilities, and a special probability for
 * the \p T domain NULL value.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */

// TODO: example for derivation from UnaryFunction, T = MyriadTuple, GenID = ?
template<typename T>
class JointPrFunction: public UnaryFunction<T, GenID>
{
public:

    /**
     * Default constructor.
     *
     * Merely creates a new function object, and does not execute any
     * initialization routines.
     */
	// TODO UnivariatePrFunction constructor unused, which private variables needed?
    JointPrFunction() :
        UnivariatePrFunction<T>(""),
        _notNullProbability(0),
        _activeDomain(nullValue<T>(), nullValue<T>()),
        _numberOfValues(0),
        _values(NULL),
        _valueProbability(0.0),
        _valueProbabilities(NULL),
        _numberOfBuckets(0),
        _buckets(0),
        _bucketProbability(0),
        _bucketProbabilities(NULL),
        _cumulativeProbabilites(NULL),
        _EPSILON(0.000001)
    {
    }

    /**
     * Anonymous file path initialization constructor.
     *
     * Loads the configuration for this probability function from the file
     * given by the \p path parameter.
     *
     * @param path The location of the function configuration file.
     */
    JointPrFunction(const string& path) :
        UnivariatePrFunction<T>(""),
        _notNullProbability(0),
        _activeDomain(nullValue<T>(), nullValue<T>()),
        _numberOfValues(0),
        _values(NULL),
        _valueProbability(0.0),
        _valueProbabilities(NULL),
        _numberOfBuckets(0),
        _buckets(0),
        _bucketProbability(0),
        _bucketProbabilities(NULL),
        _cumulativeProbabilites(NULL),
        _EPSILON(0.000001)
    {
        initialize(path);
    }

    /**
     * Destructor.
     */
    virtual ~JointPrFunction()
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
     * Returns the total number of buckets of the joint histogram.
     *
     * @return size_t The total number of buckets configured for this function.
     */
    size_t numberOfBucketsTotal() const;

    /**
       * Returns the number of buckets of dimension dim.
       *
       * @param dim The dimension for which the bucket number is requested.
       * @return size_t The number of buckets configured for this function.
    */
    size_t numberOfBuckets(size_t dim) const;


    /**
     * Returns the left bound (inclusive) of the function active domain for a given dimension.
     *
     * @param dim The dimension for which the left bound is requested.
     * @return size_t The left bound (inclusive) of the function active domain.
     */
    T min(size_t dim) const;

    /**
     * Returns the right bound (exclusive) of the function active domain for a given dimension.
     *
     * @param dim The dimension for which the right bound is requested.
     * @return size_t The right bound (exclusive) of the function active domain.
     */
    T max(size_t dim) const;

    /**
     * @see UnivariatePrFunction::operator()
     */
    Decimal operator()(const T x) const;

    /**
     * TODO: without random input, use GenID/position instead?
     * @see UnivariatePrFunction::sample()
     */
    MyriadAbstractTuple sample(Decimal random) const;

    /**
     *  not used
     * @see UnivariatePrFunction::pdf()
     */
    Decimal pdf(T x) const;

    /**
     * not used
     * @see UnivariatePrFunction::cdf()
     */
    Decimal cdf(T x) const;

    /**
     * not used
     * @see UnivariatePrFunction::invcdf()
     */
    T invcdf(Decimal x) const;

private:

    void reset();

    void normalize();

    size_t findIndex(const Decimal y) const;

    size_t findValue(const T x, bool exact = true) const;

    size_t findBucket(const T x, bool exact = true) const;

    static RegularExpression headerLine1Format;
    static RegularExpression headerLine2Format;
    static RegularExpression headerLine3Format;
    static RegularExpression valueLineFormat;
    static RegularExpression bucketLineFormat;

    Decimal _notNullProbability;

    Interval<T> _activeDomain;

    size_t _numberOfValues;
    T* _values;
    Decimal _valueProbability;
    Decimal* _valueProbabilities;

    size_t _numberOfBuckets;
    Interval<T>* _buckets;
    Decimal _bucketProbability;
    Decimal* _bucketProbabilities;

    Decimal* _cumulativeProbabilites;

    Decimal _EPSILON;
};

////////////////////////////////////////////////////////////////////////////////
/// @name Static Template Members
////////////////////////////////////////////////////////////////////////////////
//@{

// TODO: adjust regex to read multidimensional buckets (see set1.distribution for example format)
template<typename T>
RegularExpression JointPrFunction<T>::headerLine1Format("\\W*@numberofexactvals\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T>
RegularExpression JointPrFunction<T>::headerLine2Format("\\W*@numberofbins\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T>
RegularExpression JointPrFunction<T>::headerLine3Format("\\W*@nullprobability\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W*(#(.+))?");
template<typename T>
RegularExpression JointPrFunction<T>::valueLineFormat( "\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*(.+)\\W*\\}\\W*(#(.+))?");
template<typename T>
RegularExpression JointPrFunction<T>::bucketLineFormat("\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*x\\W+in\\W+\\[\\W*(.+)\\W*,\\W*(.+)\\W*\\)\\W*\\}\\W*(#(.+))?");

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Private Member Function Templates
////////////////////////////////////////////////////////////////////////////////
//@{

template<typename T>
void JointPrFunction<T>::reset()
{
    _notNullProbability = 0.0;
    _valueProbability = 0.0;
    _bucketProbability = 0.0;

    if (_numberOfValues > 0 || _numberOfBuckets > 0)
    {
        delete[] _cumulativeProbabilites;
    }

    if (_numberOfValues > 0)
    {
        _numberOfValues = 0;
        delete[] _values;
        delete[] _valueProbabilities;
    }

    if (_numberOfBuckets > 0)
    {
        _numberOfBuckets = 0;
        delete[] _buckets;
        delete[] _bucketProbabilities;
    }
}

template<typename T>
void JointPrFunction<T>::normalize()
{
    Decimal normalizationFactor = 1.0 / static_cast<Decimal>(_valueProbability + _bucketProbability + (1.0 - _notNullProbability));

    _valueProbability = 0;
    _bucketProbability = 0;

    for (size_t i = 0; i < _numberOfValues; i++)
    {
        Decimal probability = _valueProbabilities[i] * normalizationFactor;

        _valueProbabilities[i] = probability;
        _valueProbability += probability;
        _cumulativeProbabilites[i] = _valueProbability;
    }

    for (size_t i = 0; i < _numberOfBuckets; i++)
    {
        Decimal probability = _bucketProbabilities[i] * normalizationFactor;

        _bucketProbabilities[i] = probability;
        _bucketProbability += probability;
        _cumulativeProbabilites[i+_numberOfValues] = _valueProbability + _bucketProbability;
    }

    _notNullProbability = _valueProbability + _bucketProbability;
}

template<typename T>
inline size_t JointPrFunction<T>::findIndex(const Decimal y) const
{
    // we assert that the value x is in the active domain
    int min = 0;
    int max = _numberOfValues + _numberOfBuckets - 1;
    int mid = 0;

    // protect against invalid y
    if (y >= 1.0)
    {
        return max;
    }

    // continue searching while [min, max] is not empty
    while (max >= min)
    {
        // calculate the midpoint for roughly equal partition //
        mid = (min + max) / 2;

        // determine which subarray to search
        if (_cumulativeProbabilites[mid] <  y - _EPSILON)
        {
	        // change min index to search upper subarray
	        min = mid + 1;
        }
        else if (_cumulativeProbabilites[mid] > y + _EPSILON)
        {
	        // change max index to search lower subarray
	        max = mid - 1;
        }
        else
        {
	        // key found at index mid
	        // for all but the last position, increment the index by one to
	        // compensate for the fact that buckets are defined as Y < y rather than Y <= y
            return (static_cast<size_t>(mid) == _numberOfValues + _numberOfBuckets - 1) ? mid : mid+1;
        }
    }

    if (static_cast<size_t>(mid) < _numberOfValues + _numberOfBuckets - 1 && _cumulativeProbabilites[mid] < y)
    {
        return mid+1;
    }
    else
    {
        return mid;
    }
}

template<typename T>
inline size_t JointPrFunction<T>::findValue(const T x, bool exact) const
{
    // we assert that the value x is in the active domain
    int min = 0;
    int max = _numberOfValues - 1;
    int mid = 0;

    // continue searching while [min, max] is not empty
    while (max >= min)
    {
        // calculate the midpoint for roughly equal partition
        mid = (min + max) / 2;

        // determine which subarray to search
        if (_values[mid] <  x)
        {
	        // change min index to search upper subarray
	        min = mid + 1;
        }
        else if (_values[mid] > x)
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

    // at this point, mid is always equal to min and points to the first record higher than x
    // what we actually want the index of the first value less than or equal to x
    if (!exact && mid > 0)
    {
        if (static_cast<size_t>(mid) < _numberOfValues-2 && _values[mid+1] < x)
        {
	        return mid+1;
        }
        else if (_values[mid] < x)
        {
	        return mid;
        }
        else
        {
	        return mid-1;
        }
    }
    else
    {
        return nullValue<size_t>();
    }
}

template<typename T>
inline size_t JointPrFunction<T>::findBucket(const T x, bool exact) const
{
    // we assert that the value x is in the active domain
    int min = 0;
    int max = _numberOfBuckets - 1;
    int mid = 0;

    // continue searching while [min, max] is not empty
    while (max >= min)
    {
        // calculate the midpoint for roughly equal partition //
        mid = (min + max) / 2;

        // determine which subarray to search
        if (_buckets[mid].max() <=  x)
        {
	        // change min index to search upper subarray
	        min = mid + 1;
        }
        else if (_buckets[mid].min() > x)
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

    // at this point, mid is always equal to min and points to the first bucket where higher than x
    // what we actually want the index of the first bucket less than but not containing x
    if (!exact && mid > 0)
    {
        if (static_cast<size_t>(mid) < _numberOfBuckets-2 && _buckets[mid+1].max() <= x)
        {
	        return mid+1;
        }
        else if (_buckets[mid].max() <= x)
        {
	        return mid;
        }
        else
        {
	        return mid-1;
        }
    }
    else
    {
        return nullValue<size_t>();
    }
}

template<typename T>
inline Decimal JointPrFunction<T>::operator()(const T x) const
{
    return cdf(x);
}

// TODO: directly use GenID to
template<typename T>
inline T JointPrFunction<T>::sample(GenID) const
{
    return null;//invcdf(random);
}


template<typename T>
void JointPrFunction<T>::initialize(const string& path)
{
    initialize(Path(path));
}



template<typename T>
inline size_t JointPrFunction<T>::numberOfBuckets() const
{
    return _numberOfBuckets;
}

template<typename T>
inline T JointPrFunction<T>::min()const
{
    return _activeDomain.min();
}

template<typename T>
inline T JointPrFunction<T>::max() const
{
    return _activeDomain.max();
}




//@}

/** @}*/// add to math_probability group
} // namespace Myriad

#endif /* JOINTPRFUNCTION_H_ */
