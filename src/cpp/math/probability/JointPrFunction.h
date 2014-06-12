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
#include "core/types/MyriadAbstractTuple.h"
#include "math/Function.h"
#include "math/IntervalTuple.h"
#include "math/algebra/MultiplicativeGroup.h"


#include <Poco/Any.h>
#include <Poco/File.h>
#include <Poco/Path.h>
#include <Poco/String.h>
#include <Poco/RegularExpression.h>

#include <string>
#include <fstream>
#include <vector>

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
 * @author: Marie Hoffmann <marie.hoffmann@tu-berlin.de>
 */

// TODO: example for derivation from UnaryFunction, T = MyriadTuple/-Triple, genID =  const I64u
template<typename T>
class JointPrFunction //: public UnivariatePrFunction<T>
{
public:
	typedef typename T::ValueType1 V1;
	typedef typename T::ValueType2 V2;

    /**
     * Default constructor.
     *
     * Merely creates a new function object, and does not execute any
     * initialization routines.
     */
    JointPrFunction() :
    	//UnivariatePrFunction<T>(""),
        _activeDomain(NULL, NULL), //nullValue<MyriadTuple>(), nullValue<MyriadTuple>()), // TODO: lower, upper bin edges multidimensional Interval needed
        _numberOfBuckets(0),
        _bucketProbabilities(NULL),
        _buckets(NULL),
        _cardinalities(NULL),
        _sampleSize(0),
        _dim(2)
        //_cumulativeProbabilites(NULL),
        //_EPSILON(0.000001)
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
    	  //UnivariatePrFunction<T>(""),
    	 _activeDomain(nullValue<T>(), nullValue<T>())//, //nullValue<MyriadTuple>(), nullValue<MyriadTuple>()), // TODO: lower, upper bin edges multidimensional Interval needed
//    	 _numberOfBuckets(0),
//    	 _bucketProbabilities(NULL),
//    	 _buckets(NULL),
//    	 _cardinalities(NULL),
//         _sampleSize(0),
//         _dim(2)
    {
        initialize(path);
    }

    /**
     * Destructor.
     */
    // TODO: add reset function
    /*virtual ~JointPrFunction()
    {
        reset();
    }*/

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
      * Initializes the function with the configuration stored in the file
      * located at the given \p path.
      *
      * @param path The location of the function configuration file.
      */
     void initialize(const Path& path);

     /**
      * Initialization routine.
      *
      * Initializes the function with the configuration from the input stream
      * given by the \p in parameter.
      *
      * @param in Input stream containing the function configuration.
      */
     void initialize(istream& in);

     /**
      * Initialization routine.
      *
      * Initializes the function with the configuration from the input stream
      * given by the \p in parameter. When reading from the \p in stream, uses
      * the \p currentLineNumber parameter to track the current line number.
      *
      * @param currentLineNumber A reference to the current line number.
      * @param in Input stream containing the function configuration.
      */
     void initialize(istream& in, I16u& currentLineNumber);

    /**
     * Returns the total number of buckets of the joint histogram.
     *
     * @return size_t The total number of buckets configured for this function.
     */
    size_t numberOfBuckets() const;

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
  //  Decimal operator()(const T x) const;

    /**
     * TODO: without random input, use GenID/position instead?
     * @see UnivariatePrFunction::sample()
     */
    T sample(const I64u GenID) const;


private:

    void reset();
//
//    void normalize();

    I64u findIndex() const;

    I64u findValue() const;

    I64u findBucket(const I64u _tupleID) const;

    I64u normalizeTupleID(I64u tID, size_t bID) const;

    I64u permuteTupleID(I64u tID) const;

    I64u permuteTupleID(I64u tID, size_t bID) const;

    T scalar2Tuple(I64u tID, I64u bID) const;

    static RegularExpression headerLine1Format;
    static RegularExpression headerLine2Format;
    static RegularExpression headerLine3Format;
    static RegularExpression valueLineFormat;
    static RegularExpression bucketLineFormat;

    // TODO: IntervalTuple generic

    IntervalTuple<T> _activeDomain; 		// attribute domain as interval [min(_buckets), max(_buckets)]
    size_t _numberOfBuckets;				// of multidimensional histogram
    IntervalTuple<T>* _buckets; 	// buckets of joint histogram, format [((low_T1, low_Tuple2), (up_T1, up_T2))]
    Decimal* _bucketProbabilities;			// of sequentially ordered multidimensional buckets
    vector<I64u*> _cardinalities;    // cardinality per bucket for each dimension, format [[card_T1_Bi, card_T2_Bi]]_i, T = type, B = bucket index
    I64u _sampleSize; 				// total table size
    size_t _dim;					// dimensionality of histogram = attribute number of composite key
};

////////////////////////////////////////////////////////////////////////////////
/// @name Static Template Members
////////////////////////////////////////////////////////////////////////////////
//@{

// TODO: adjust regex to read multidimensional buckets (see set1.distribution for example format)
//template<typename T>
//RegularExpression JointPrFunction<T>::headerLine1Format("\\W*@numberofexactvals\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
template<typename T>
RegularExpression JointPrFunction<T>::headerLine2Format("\\W*@numberofbins\\W*=\\W*([+]?[0-9]+)\\W*(#(.+))?");
// TODO: allow yes/no nullprobs
//template<typename T>
//RegularExpression JointPrFunction<T>::headerLine3Format("\\W*@nullprobability\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W*(#(.+))?");
// TODO: allow yes/no single values
//template<typename T>
//RegularExpression JointPrFunction<T>::valueLineFormat( "\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*(.+)\\W*\\}\\W*(#(.+))?");
//RegularExpression JointPrFunction<T>::bucketLineFormat("\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*x\\W+in\\W+\\[\\W*(.+)\\W*,\\W*(.+)\\W*\\)\\W*\\}\\W*(#(.+))?");
// TODO: more generic, currently matches only tuples
template<typename T>
RegularExpression JointPrFunction<T>::bucketLineFormat("\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*x\\W+in\\W+\\[\\W*\\[(.+)\\W*,\\W*(.+)\\W*\\)\\W*,\\W*\\[\\W*(.+)\\W*,\\W*(.+)\\W*\\)\\W*]\\W*\\}\\W*(#(.+))?");
//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Private Member Function Templates
////////////////////////////////////////////////////////////////////////////////
//@{

template<typename T>
void JointPrFunction<T>::reset()
{
    _numberOfBuckets = 0;
    delete[] _bucketProbabilities;
    delete[] _buckets;
    _cardinalities.clear();
    _sampleSize = 0;
    _dim = 0;
}

//// TODO: ?
//template<typename T>
//inline Decimal JointPrFunction<T>::operator()(const T x) const
//{
//    return Decimal(0);
//}

/*
template<typename T>
void JointPrFunction<T>::normalize()
{
    Decimal normalizationFactor = 1.0 / static_cast<Decimal>(_valueProbability + _bucketProbability + (1.0 - ));

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
}*/

template<typename T>
inline I64u JointPrFunction<T>::findIndex() const
{
	I64u index = 0;
	return index;
}


/*
 * Assign bucket relative position of genID within total sequence size
 */
template<typename T>
inline I64u JointPrFunction<T>::findBucket(I64u tupleID) const
{
	I64u bucketID = -1;
	Decimal s = 0;
	for (size_t i = 0; i < _numberOfBuckets; ++i){
		s += _bucketProbabilities[i];
		if (ceil(_sampleSize*s) <= tupleID){
			bucketID = i;
			break;
		}
	}
	return bucketID;
}

/*
 * Adjust _tupleID to lower bin edge by substracting lowest _tupleID for this bucket.
 */
template<typename T>
inline I64u JointPrFunction<T>::normalizeTupleID(I64u tupleID, size_t bucketID) const
{
	Decimal s = 0;
	for (size_t i = 0; i < bucketID; ++ i)
		s += _bucketProbabilities[i];
	I64u tupleID_left = ceil(s*_sampleSize) + 1;
	return tupleID-tupleID_left;
}

/*
 * Shuffle _tupleID <- [0; _sampleSize]
 */
template<typename T>
inline I64u JointPrFunction<T>::permuteTupleID(I64u tupleID) const
{
	MultiplicativeGroup mg(_sampleSize);
	return mg(tupleID);
}

/*
 * Permute bucket's tupleID to larger index in [0; card(bucketID)-1]
 */
template<typename T>
inline I64u JointPrFunction<T>::permuteTupleID(I64u tupleID, size_t bucketID) const
{
	I64u cardinality = 1;
	for (size_t i = 0; i < _dim; ++i)
		cardinality *= _cardinalities.at(bucketID)[i];
	MultiplicativeGroup mg(cardinality);
	return mg(tupleID);
}

/*
 * Scalar tuple identifier is mapped to output tuple dimension-wise
 * */
template<typename T>
inline T JointPrFunction<T>::scalar2Tuple(I64u tupleID, I64u bucketID) const
{
	// transform scalar index to dimension-wise indices
	I64u compositeID[_dim];
	I64u gamma; // cardinality of subsequent dimensions in same bucket
	I64u rem = tupleID;
	for (size_t i = 0; i < _dim-1; ++i){
		gamma = 1;
		for (size_t j = i+1; j < _dim; ++j)
			gamma *= _cardinalities.at(bucketID)[j];
		compositeID[i] = rem/gamma; // div rounds to floor
		rem = rem % gamma;
	}
	compositeID[_dim-1] = rem;
	// map indices to attribute values, use Interval

	// create new Tuple/Triple instance
	// if dim = 2  T = MyriadTuple(ValueType1 val1, ValueType2 val2)
	T t = new T();
	return t;
}


// TODO: directly use GenID to
template<typename T>
inline T JointPrFunction<T>::sample(const I64u genID) const
{
	I64u tupleID = genID;

	// TODO: 1. permute tupleID <- [0;_sampleSize-1], should be reset globally
	//tupleID = permuteTupleID(tupleID);

	// 2. find bucket
	I64u bucketID = findBucket(tupleID);

	// 3. normalize to [0, _bucketProbabilities[bucketID]*_sampleSize]
	tupleID = normalizeTupleID(tupleID, bucketID);

	// 4. permute tuple index <- [0, card(bucket)-1]
	tupleID = permuteTupleID(tupleID, bucketID);

	// 5. transform scalar tuple index into tuple of indices
	T t = scalar2Tuple(tupleID, bucketID);

    return t;
}


template<typename T>
void JointPrFunction<T>::initialize(const string& path)
{
    initialize(Path(path));
}

template<typename T>
void JointPrFunction<T>::initialize(const Path& path)
{
    if (!path.isFile())
    {
        throw ConfigException(format("Cannot find file at `%s`", path.toString()));
    }

    File file(path);

    if (!file.canRead())
    {
        throw ConfigException(format("Cannot read from file at `%s`", path.toString()));
    }

    ifstream in(file.path().c_str());

    if (!in.is_open())
    {
        throw ConfigException(format("Cannot open file at `%s`", path.toString()));
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

template<typename T>
void JointPrFunction<T>::initialize(istream& in)
{
    I16u currentLineNumber = 1;
    initialize(in, currentLineNumber);
}
//template< template<class S1, class S2> class T >
template<typename T>
void
JointPrFunction<T>::initialize(istream& in, I16u& currentLineNumber)
{
    enum READ_STATE { NOE, NOB, NPR, VLN, BLN, FIN, END };

    // reset old state
    reset();

    // reader variables
    READ_STATE currentState = NOE; // current reader machine state
    string currentLine; // the current line
    I16u currentItemIndex = 0; // current item index
    RegularExpression::MatchVec posVec; // a posVec for all regex matches

    // reader finite state machine
    while (currentState != END)
    {
        // the special FIN stage contains only final initialization constructs
        // and does not a currentLine
        if (currentState == FIN)
        {
        	// TODO: extract min/max from buckets AND exact _values and check sorted ordering
	        //T min = std::min<T>(_buckets[0].min(), _values[0]);
        	//T max = std::max<T>(_buckets[_numberOfBuckets-1].max(), static_cast<T>(_values[_numberOfValues-1]+1));

        	T min = _buckets[0].min(); // left bin edges of left most Interval
        	T max = _buckets[_numberOfBuckets-1].max(); // right bin edges of right most Interval

	        _activeDomain.set(min, max); //_buckets[_numberOfBuckets-1]);

	        currentState = END;
	        continue;
        }

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

        /*if (currentState == NOE)
        {
	        if (!in.good() || !headerLine1Format.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofexactvals = ' + x", currentLineNumber, currentLine));
	        }

	        I32 numberOfValues = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

	        if (numberOfValues <= 0 && numberOfValues > 65536)
	        {
		        throw DataException("Invalid number of exact values '" + toString(numberOfValues) +  "'");
	        }

	        _numberOfValues = numberOfValues;
	        _values = new T[numberOfValues];
	        _valueProbabilities = new Decimal[numberOfValues];

	        currentState = NOB;
        }
        else */
        if (currentState == NOB)
        {
	        if (!in.good() || !headerLine2Format.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofbins = ' + x", currentLineNumber, currentLine));
	        }

	        I32 numberOfBuckets = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

	        if (numberOfBuckets <= 0 && numberOfBuckets > 65536)
	        {
		        throw DataException("Invalid number of buckets '" + toString(numberOfBuckets) +  "'");
	        }

	        _numberOfBuckets = numberOfBuckets; // ok
	        _buckets = new IntervalTuple<T>[numberOfBuckets];
	        _bucketProbabilities = new Decimal[numberOfBuckets];


	        currentState = BLN;
	     //   currentState = NPR;
        }
/*        else if (currentState == NPR)
        {
	        if (!in.good() || !headerLine3Format.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@nullprobability = ' + x", currentLineNumber, currentLine));
	        }

	        _notNullProbability = 1.0 - atof(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

	        currentItemIndex = 0;
	        currentState = (_numberOfValues > 0) ? VLN : BLN;
        }
        else if (currentState == VLN)
        {
	        if (!in.good() || !valueLineFormat.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad value probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = {' + x + ' }'", currentLineNumber, currentLine));
	        }

	        Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
	        T value = fromString<T>(currentLine.substr(posVec[3].offset, posVec[3].length));

	        _values[currentItemIndex] = value;
	        _valueProbabilities[currentItemIndex] = probability;
//	        _valueProbability += probability;
//	        _cumulativeProbabilites[currentItemIndex] = _valueProbability;

	        currentItemIndex++;

	        if (currentItemIndex >= _numberOfValues)
	        {
		        currentState = (_numberOfBuckets > 0) ? BLN : FIN;
		        currentItemIndex = 0;
	        }
        }
        else */
        if (currentState == BLN)
        {
	        if (!in.good() || !bucketLineFormat.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad bucket probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = { x \\in [min_i, max_i)_i=1..d }'", currentLineNumber, currentLine));
	        }

	        // TODO: rewrite T initialization to fit for triples
	        Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
	        VType1 min1 = fromString<VType1>(currentLine.substr(posVec[3].offset, posVec[3].length));
	        typename T::ValueType1 max1 = fromString<T::ValueType1>(currentLine.substr(posVec[4].offset, posVec[4].length));

	        typename T::ValueType2 min2 = fromString<T::ValueType2>(currentLine.substr(posVec[5].offset, posVec[5].length));
	        typename T::ValueType2 max2 = fromString<T::ValueType2>(currentLine.substr(posVec[6].offset, posVec[6].length));

	        T min = new T(min1, min2);
	        T max = new T(max1, max2);

	        _buckets[currentItemIndex].set(min, max);
	        _bucketProbabilities[currentItemIndex] = probability;
	    //    _bucketProbability += probability;
	    //    _cumulativeProbabilites[currentItemIndex+_numberOfValues] = _valueProbability + _bucketProbability;

	        currentItemIndex++;

	        if (currentItemIndex >= _numberOfBuckets)
	        {
		        currentState = FIN;
		        currentItemIndex = 0;
	        }
        }

        currentLineNumber++;
    }

    // protect against unexpected reader state
    if (currentState != END)
    {
        throw RuntimeException("Unexpected state in JointPrFunction reader at line " + currentLineNumber);
    }

    // check if extra normalization is required
    // TODO
   /*if (std::abs(_valueProbability + _bucketProbability - _notNullProbability) >= 0.00001)
    {
        normalize();
    }*/
}

template<typename T>
inline size_t JointPrFunction<T>::numberOfBuckets() const
{
    return _numberOfBuckets;
}

//template<typename Tuple>
//inline Tuple JointPrFunction<Tuple>::min()const
//{
//    return _activeDomain.min();
//}
//
//template<typename Tuple>
//inline Tuple JointPrFunction<Tuple>::max() const
//{
//    return _activeDomain.max();
//}




//@}

/** @}*/// add to math_probability group
} // namespace Myriad

#endif /* JOINTPRFUNCTION_H_ */
