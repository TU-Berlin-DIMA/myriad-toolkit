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

#include "core/constants.h"
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
#include <limits>
#include <math.h>
//#include <cmath>

#include <stdlib.h>
#include <algorithm>
#include <random>

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
class JointPrFunction: public AbstractFunction
{
public:
	template<typename S> friend class JointPrFunctionTest; // runtime exception when trying to access private functions/members

	typedef typename T::VType1 V1;
	typedef typename T::VType2 V2;

	/**
     * Default constructor.
     *
     * Merely creates a new function object, and does not execute any
     * initialization routines.
     */
    JointPrFunction(const string& name) :
    	AbstractFunction(name),
//        _activeDomain(nullValue<T>(), nullValue<T>()), // TODO: lower, upper bin edges multidimensional Interval needed
        _numberOfBuckets(0),
//        _bucketProbabilities(nullValue<Decimal*>()),
////        _cardinalities(NULL),
        _sampleSize(8), // <- for testing only
		_numberOfValues(0),
		_generator(MT19937_64)
//        //_cumulativeProbabilites(NULL),
//        //_EPSILON(0.000001)
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
    JointPrFunction(const string& name, const string& path) :
    	AbstractFunction(name),
//    	 _activeDomain(nullValue<T>(), nullValue<T>()),//, //nullValue<MyriadTuple>(), nullValue<MyriadTuple>()), // TODO: lower, upper bin edges multidimensional Interval needed
    	 _numberOfBuckets(0),
//    	 _bucketProbabilities(nullValue<Decimal*>()),
    	 _sampleSize(8),	// <- for testing only
    	 _numberOfValues(0),
 		_generator(MT19937_64)
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
     * Returns histogram
     */
    vector<IntervalTuple<T> > getBuckets() const;

    vector<Decimal> getBucketProbabilities() const;


    /**
     * Returns the left bound (inclusive) of the function active domain.
     *
     * @param dim The dimension for which the left bound is requested.
     * @return size_t The left bound (inclusive) of the function active domain.
     */

    T min() const;

    /**
     * Returns the right bound (exclusive) of the function active domain for a given dimension.
     *
     * @param dim The dimension for which the right bound is requested.
     * @return size_t The right bound (exclusive) of the function active domain.
     */
    T max() const;

    /**
     * @see UnivariatePrFunction::operator()
     */
  //  Decimal operator()(const T x) const;

    /**
     * TODO: without random input, use GenID/position instead?
     * @see UnivariatePrFunction::sample()
     */
    T sample(const I64u GenID, const I64u sampleSize);

    double cdf(const I64u _binID);

    /*
     * Assign unique tuple identifier to a bin with edges adjusted to specified domainSize
     * max(tupleID) = domainSize-1
     * */
    I64u findBucket(const I64u tupleID);



    void reset();

    void setSampleSize(I64u sampleSize);

    I64u normalizeTupleID(I64u tID, I64u bID) ;

    I64u permuteTupleID(I64u tID, I64u bID);

    I64u permuteSampleID(I64u sampleID);

    T scalar2Tuple(I64u tID, I64u bID) ;

    void setGenerator(GENERATOR generator) ;

private:

    static RegularExpression headerLine1Format;
    static RegularExpression headerLine2Format;
    static RegularExpression headerLine3Format;
    static RegularExpression valueLineFormat;
    static RegularExpression bucketLineFormat;

    // TODO: IntervalTuple generic

    IntervalTuple<T> _activeDomain; 		// attribute domain as interval [min(_buckets):MyriadTuple, max(_buckets):MyriadTuple]
    I32 _numberOfBuckets;					// of multidimensional histogram
    vector<IntervalTuple<T> > _buckets; 	// buckets of joint histogram, format [((low_T1, low_Tuple2), (up_T1, up_T2))]
    vector<Decimal> _bucketProbabilities;	// of sequentially ordered multidimensional buckets
   // vector<vector<double> > _cardinalities;    		// cardinality per bucket for each dimension, format [[card_T1_Bi, card_T2_Bi]]_i, T = type, B = bucket index
    I64u _sampleSize; 						// total table size
    size_t _dim;							// dimensionality of histogram = attribute number of composite key
    I32 _numberOfValues;
    //T* _values;
    Decimal _valueProbability;
    Decimal* _valueProbabilities;			// probabilities of exact values
    Decimal _notNullProbability;
    Decimal _bucketProbability;
    vector<Decimal> _cumulativeProbabilities;
    vector<I64u> _rangeMap;					// TODO: initialize when xml spec given (needs sampleSize + distribution file), e.g. in init()
    GENERATOR _generator;
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
// TODO: allow yes/no nullprobs
template<typename T>
RegularExpression JointPrFunction<T>::headerLine3Format("\\W*@nullprobability\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W*(#(.+))?");
// TODO: allow yes/no single values
template<typename T>
RegularExpression JointPrFunction<T>::valueLineFormat( "\\W*p\\(X\\)\\W*=\\W*([+]?[0-9]*\\.?[0-9]+([eE][-+]?[0-9]+)?)\\W+for\\W+X\\W*=\\W*\\{\\W*(.+)\\W*\\}\\W*(#(.+))?");
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
	//cout << "entered reset ..." << endl;
	// _activeDomain.reset()
	_buckets.clear();
    _bucketProbabilities.clear();
    _sampleSize = 8;
    _dim = 2;
    _numberOfBuckets = 0;
    _valueProbability = 0.0;
    _bucketProbability = 0.0;
    _cumulativeProbabilities.clear();
    _rangeMap.clear();
    //cout << "... leaving reset" << endl;
}

template<typename T>
inline void JointPrFunction<T>::setSampleSize(I64u sampleSize){
	_sampleSize = sampleSize;
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


/*
 * Assign bucket relative position of genID within total sequence size
 */
//template<typename T>
//inline I64u JointPrFunction<T>::findBucket(I64u tupleID) const
//{
//	I64u bucketID = -1;
//	Decimal cs = 0;
//	for (size_t i = 0; i < _numberOfBuckets; ++i){
//		cs += _bucketProbabilities[i];
//		if (ceil(_sampleSize*cs) < tupleID){
//			bucketID = i;
//			break;
//		}
//	}
//	if (bucketID < 0) throw LogicalException("Index error for bucketID in JointPrFunction::findBucket()");
//	return bucketID;
//}

// TODO: compute step function once for each node not for each tupleID
// pre: GenID in [0.._sampleSize-1]
template<typename T>
inline I64u JointPrFunction<T>::findBucket(I64u tupleID)
{
   double eps = 100*numeric_limits<double>::epsilon();
   // construct step function map if not done previously
   if (_rangeMap.size() == 0){
	   for (unsigned int i = 0; i < _numberOfBuckets; ++i)
		   _rangeMap.push_back((I64u) round(this->_cumulativeProbabilities.at(i)*this->_sampleSize));

	   // check whether assigned ranges do not exceed bin's cardinality
	   I64 delta = 0;
	   I64 curr;
	   for (unsigned int run = 0; run < _numberOfBuckets; ++run){
		   for (unsigned int i = 0; i<_numberOfBuckets; ++i){
			   // compute current range size
			   if (i == 0)
				   curr = _rangeMap.at(i);
			   else
				   curr = _rangeMap.at(i)-_rangeMap.at(i-1);
			   // increment successor's range if assigned range exceeds cardinality of current bin
			   delta = curr - _buckets.at(i).length();
			   if (delta > 0){ // exceeding!
				   if (i < _numberOfBuckets-1){ // correct upper bin edge and continue
					   _rangeMap.at(i) = _rangeMap.at(i) - delta;
				   }
				   else{  // shift upper bounds of all predecessors => bin 0 becomes enlarged
					  // TODO: test whether last right bin edge does not exceed _sampleSize
					   for (unsigned int j = 0; j < _numberOfBuckets-1; ++j)
						   _rangeMap.at(j) += delta;
				   }
			   }
		   }
	   }
	}

   // larger computational time for backmost tuple ids, use something else hash/modulo/linked lists/...
   vector<I64u>::iterator it = find_if (_rangeMap.begin(), _rangeMap.end(), [&tupleID](I64u i){return i>tupleID;} );
   I64u bID = it - _rangeMap.begin();

   return bID;
}

/*
 * Adjust _tupleID to lower bin edge by subtracting lowest _tupleID for this bucket.
 */
template<typename T>
inline I64u JointPrFunction<T>::normalizeTupleID(I64u tupleID, I64u bucketID)
{
	if (_rangeMap.size() == 0) // compute step function
		findBucket(tupleID);
	I64u tID = (bucketID == 0) ? tupleID : tupleID - _rangeMap.at(bucketID-1);
	return tID;
}


/*
 * Permute bucket's tupleID to (larger) index in [0; card(bucketID)-1]
 * FIXME: test range 0..p-1 or 1..p-1 of gen
 */
template<typename T>
inline I64u JointPrFunction<T>::permuteTupleID(I64u tupleID, I64u bucketID)
{
	I64u gamma = this->_buckets.at(bucketID).length();
	I64u nextPower = pow(2,ceil(log2(gamma)));
//	cout << "gamma = " << gamma << ", log2(gamma) = " << log2(gamma) << ", ceil(log2(gamma)) = "<< ceil(log2(gamma)) << ", next Power = " << nextPower << endl;
	//I64u pad = rand() % nextPower;

	// default_random_engine, knuth_b, minstd_rand, minstd_rand0, mt19937, mt19937_64, ranlux24, ranlux48
	srand(1);
	I64u seed = rand(); // 0 as seed bad for mt
	I64u tID;
	uniform_int_distribution<I64u> distribution(0,nextPower-1);

	switch(this->_generator){
		case DEFAULT_RANDOM_ENGINE:
		{
			default_random_engine g(seed);
			tID = tupleID^distribution(g);
		}
		break;
		case KNUTH_B:
		{
			knuth_b g(seed);
			tID = tupleID^distribution(g);
		}
		break;
		case MINSTD_RAND:
		{
			minstd_rand g(seed);
			tID = tupleID^distribution(g);

		}
		break;
		case MINSTD_RAND0:
		{
			minstd_rand0 g(seed);
			tID = tupleID^distribution(g);
		}
			break;
		case MT19937:
		{
			mt19937 g(seed);
			tID = tupleID^distribution(g);
		}
			break;
		case MT19937_64:
		{
			mt19937_64 g(seed);
			tID = tupleID^distribution(g);
		}
			break;
		case RANLUX24:
		{
			ranlux24 g(seed);
			tID = tupleID^distribution(g);
		}
			break;
		case RANLUX48:
		{
			ranlux48 g(seed);
			tID = tupleID^distribution(g);
		}
			break;
		case RAND:{
			srand(1);
			I64u pad = rand();
			pad %= nextPower;
			tID =  tupleID^pad;
		}
			break;
		case IDENT:{
			tID = tupleID;
		}
			break;
		default:
			break;

	}
	return tID % gamma;
}

/**
 * Permute GenID in [1..sampleSize] using Mersenne-Twister to generate a pad for Xoring.
 * If sampleSize is not a power of 2, computing modulo is only a hack and might produce duplicates.
 *
 */
template<typename T>
inline I64u JointPrFunction<T>::permuteSampleID(I64u sampleID)
{
	//cout << "permuteSampleID 1 with sampleID: " << sampleID << endl;
	I64u nextPower = pow(2,ceil(log2(this->_sampleSize)));
	//cout << "permuteSampleID 2" << endl;
	srand(2);
	I64u seed = rand(); // 0 as seed bad for mt
	//cout << "permuteSampleID 3 with nextPower: " << nextPower << endl;
	uniform_int_distribution<I64u> distribution(0,nextPower-1);
	//cout << "permuteSampleID 4" << endl;
	mt19937 g(seed);
	//cout << "permuteSampleID 5" << endl;
	I64u pad = distribution(g);
	//cout << "permuteSampleID 6" << endl;
	I64u sID = (sampleID^pad);
	//cout << "permuteSampleID 7" << endl;
	sID %= this->_sampleSize;
	//cout << "permuteSampleID 8" << endl;
	return sID;
}


/*
 * Scalar tuple identifier is mapped to output tuple dimension-wise
 * TODO: works currently only for tuples of two items
 * */
template<typename T>
inline T JointPrFunction<T>::scalar2Tuple(I64u tupleID, I64u bucketID)
{
	if (bucketID > this->numberOfBuckets()) throw LogicalException("Requested bucketID in JointPrFunction exceeds histogram size!");
	// transform scalar index to dimension-wise indices
	I64 compositeID[_dim];
	I64 gamma = 1; // cardinality of subsequent dimensions in same bucket
	I64 rem = tupleID;
	gamma *= _buckets.at(bucketID).length(1);

	// position in attribute domain A_i
	I64u pos = rem/gamma;
	compositeID[0] = pos + _buckets.at(bucketID).min().getFirst();
	rem = rem % gamma;
	pos = rem;
	compositeID[_dim-1] = pos+_buckets.at(bucketID).min().getSecond();

	// map indices to attribute values, use Interval
	// create new Tuple/Triple instance
	// if dim = 2  T = MyriadTuple(ValueType1 val1, ValueType2 val2)
	// TODO: how to convert I64u values into Domain values?
	// compute index positions within single attributes
	I32u gamma1 = 0, gamma2 = 0;
	for (unsigned int l = 0; l < bucketID; ++l){
		gamma1 += _buckets.at(l).length(0);
		gamma2 += _buckets.at(l).length(1);

	}
	//T t1(_activeDomain.min().getFirst() + gamma1 + compositeID[0], _activeDomain.min().getSecond() + gamma2 + compositeID[1]);
	T t(compositeID[0], compositeID[1]);
	return t;
}

//
template<typename T>
void JointPrFunction<T>::setGenerator(GENERATOR generator){
	this->_generator = generator;
}


// TODO: directly use GenID to
template<typename T>
inline T JointPrFunction<T>::sample(const I64u genID, const I64u sampleSize)
{
	//cout << "sample 1" << endl;
	this->_sampleSize = sampleSize;
	//cout << "sample 2" << endl;
	// 1. permute tupleID <- [0;pow(2,floor(log2(_sampleSize)))-1]
	I64u tupleID = permuteSampleID(genID);
	// 2. find bucket
	//cout << "sample 3" << endl;
	I64u bucketID = findBucket(tupleID);
	// 3. normalize to [0, _bucketProbabilities[bucketID]*_sampleSize]
	//cout << "sample 4" << endl;
	tupleID = normalizeTupleID(tupleID, bucketID);
	// 4. permute tuple index <- [0, card(bucket)-1]
	//cout << "sample 5" << endl;
	tupleID = permuteTupleID(tupleID, bucketID);
	// 5. transform scalar tuple index into tuple of indices
	//cout << "sample 6" << endl;
	T t = scalar2Tuple(tupleID, bucketID);
	//cout << "sample 7" << endl;
	return t;
}

template<typename T>
double JointPrFunction<T>::cdf(const I64u _binID)
{
	if (0 >_binID || _binID >= _numberOfBuckets)
		throw ConfigException("Unexpected bucketID: out of range");
	return this->_cumulativeProbabilities.at(_binID);
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
// TODO: set _dim here
template<typename T>
void
JointPrFunction<T>::initialize(istream& in, I16u& currentLineNumber)
{
	enum READ_STATE { NOE, NOB, NPR, VLN, BLN, FIN, END };

    // reset old state
    reset();

    // reader variables
    READ_STATE currentState = NOE; // current reader machine state, state: NOE (number of exact values) omitted
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
        	//	cout << "entered state FIN" << endl;
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
        if (currentState == NOE)
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
	        //_values = new T[numberOfValues];
	        //_valueProbabilities = new Decimal[numberOfValues];

	        currentState = NOB;
        }

        else if (currentState == NOB)
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

	        /* TODO: initialize bin, bin probabilities */
	        _numberOfBuckets = numberOfBuckets;
	        // TODO: more sophisticated vector initialization?
	        for (I32u i = 0; i < _numberOfBuckets; ++i) _cumulativeProbabilities.push_back(0.0);
	        currentState = NPR;
	    }
        else if (currentState == NPR)
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
        	throw DataException("Entered state VLN, but exact values currently not supported!");
            if (!in.good() || !valueLineFormat.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad value probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = {' + x + ' }'", currentLineNumber, currentLine));
	        }


	        //Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
	        // TODO: fromString does not work for MyriadTuple
	        // T value = fromString<T>(currentLine.substr(posVec[3].offset, posVec[3].length));

	        //_values[currentItemIndex] = value;
	        //_valueProbabilities[currentItemIndex] = probability;
	        //_valueProbability += probability;
	      //  _cumulativeProbabilities.at(currentItemIndex) = _valueProbability;

	        //cout << "_cdfProb [" << currentItemIndex << "] = " << _cumulativeProbabilities.at(currentItemIndex) << endl;

	        currentItemIndex++;

	        if (currentItemIndex >= _numberOfValues)
	        {
		        currentState = (_numberOfBuckets > 0) ? BLN : FIN;
		        currentItemIndex = 0;
	        }

        }
        else if (currentState == BLN)
        {
            if (!in.good() || !bucketLineFormat.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad bucket probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = { x \\in [min_i, max_i)_i=1..d }'", currentLineNumber, currentLine));
	        }

            //for (int i = 1; i < 7; ++i)
	        //cout << currentLine.substr(posVec[1].offset, posVec[1].length) << endl;
	        // TODO: rewrite T initialization to fit for triples
	        Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
	        V1 min1 = fromString<V1>(currentLine.substr(posVec[3].offset, posVec[3].length));
	        V1 max1 = fromString<V1>(currentLine.substr(posVec[4].offset, posVec[4].length));

	        V2 min2 = fromString<V2>(currentLine.substr(posVec[5].offset, posVec[5].length));
	        V2 max2 = fromString<V2>(currentLine.substr(posVec[6].offset, posVec[6].length));

	        T min(min1, min2);
	        T max(max1, max2);
	        IntervalTuple<T> interval(min, max); // set new interval as a set of lower and upper bin edges

	        _buckets.push_back(interval); // .at(currentItemIndex).set(min, max);
	        //cout << "init: buckets += " << "("<< min1 << ", " << max1 << ", (" << min2 << ", " << max2 << ")\n";
	        _bucketProbabilities.push_back(probability);
	        _dim = 2; // TODO: more generic version - extract from regex
	        _bucketProbability += probability;

	        _cumulativeProbabilities.at(currentItemIndex+_numberOfValues) = _valueProbability + _bucketProbability;
	        //cout << "_cdfProb [" << currentItemIndex << "] = " << _cumulativeProbabilities.at(currentItemIndex) << ", valueProb = " << _valueProbability << ", buckProb = " << _bucketProbability<< endl;
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
        throw RuntimeException(format("Unexpected state in JointPrFunction reader at line %d", currentLineNumber));
    }

    // check if extra normalization is required
    // TODO
   /*if (std::abs(_valueProbability + _bucketProbability - _notNullProbability) >= 0.00001)
    {
        normalize();
    }*/
//    cout << "Leaving initialize" << endl;
}

template<typename T>
inline size_t JointPrFunction<T>::numberOfBuckets() const
{
    return _numberOfBuckets;
}

template<typename T>
inline vector<IntervalTuple<T> > JointPrFunction<T>::getBuckets() const
{
    return _buckets;
}

template<typename T>
inline vector<Decimal> JointPrFunction<T>::getBucketProbabilities() const
{
    return _bucketProbabilities;
}



// return tuple of lower bin edges
template<typename T>
inline T JointPrFunction<T>::min()const
{
    return _activeDomain.min();
}

// return tuple of upper bin edges
template<typename T>
inline T JointPrFunction<T>::max()const
{
    return _activeDomain.max();
}


//@}

/** @}*/// add to math_probability group
} // namespace Myriad

#endif /* JOINTPRFUNCTION_H_ */
