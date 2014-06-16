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
#include <limits>
#include <math.h>

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
	template<typename S> friend class JointPrFunctionTest;

	typedef typename T::VType1 V1;
	typedef typename T::VType2 V2;

	/**
     * Default constructor.
     *
     * Merely creates a new function object, and does not execute any
     * initialization routines.
     */
    JointPrFunction() :
    	//UnivariatePrFunction<T>(""),
//        _activeDomain(nullValue<T>(), nullValue<T>()), // TODO: lower, upper bin edges multidimensional Interval needed
        _numberOfBuckets(0),
//        _bucketProbabilities(nullValue<Decimal*>()),
////        _cardinalities(NULL),
//        _sampleSize(0),
		_numberOfValues(0),
		_sampleSize(8)  // <- for testing only
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
    JointPrFunction(const string& path) :
    	  //UnivariatePrFunction<T>(""),
//    	 _activeDomain(nullValue<T>(), nullValue<T>()),//, //nullValue<MyriadTuple>(), nullValue<MyriadTuple>()), // TODO: lower, upper bin edges multidimensional Interval needed
    	 _numberOfBuckets(0),
//    	 _bucketProbabilities(nullValue<Decimal*>()),
//    	 _sampleSize(0),
    	 _numberOfValues(0),
    	 _sampleSize(8) // <- for testing only
    {
    	cout << "JointPrFunction() with path = " << path << endl;
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
    T sample(const I64u GenID) const;

    double cdf(const I64u _binID) const;

    I64u findBucket(const I64u _tupleID) const;

private:

    void reset();
//
//    void normalize();

    I64u normalizeTupleID(I64u tID, size_t bID) const;

    I64u permuteSampleID(I64u tID) const;

    I64u permuteTupleID(I64u tID, size_t bID) const;

    T scalar2Tuple(I64u tID, I64u bID) const;

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
	cout << "entered reset ..." << endl;
	// _activeDomain.reset()
	_buckets.clear();
    _bucketProbabilities.clear();
    _sampleSize = 8;
    _dim = 2;
    _numberOfBuckets = 0;
    _valueProbability = 0.0;
    _bucketProbability = 0.0;
    _cumulativeProbabilities.clear();
    cout << "... leaving reset" << endl;
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
inline I64u JointPrFunction<T>::findBucket(I64u tupleID) const
{
I64u bucketID = 0;
   double eps = 100*numeric_limits<double>::epsilon();
   // construct step function map
   vector<I64u> rangeMap;
   cout << "expected ranges with n = "<<_sampleSize << endl;
   for (unsigned int i = 0; i < _numberOfBuckets; ++i){
	   rangeMap.push_back((I64u) round(this->_cumulativeProbabilities.at(i)*_sampleSize+eps));
	   cout << "\t" << rangeMap.back();
   }
   // check whether assigned ranges do not exceed bin's cardinality
   I64 delta = 0;
   I64 curr;

   /*
    *     rangeMap = arrayfun(@(phi) round(phi*n+eps), phi_cum)
    % check whether assigned ranges do not exceed bin's cardinality
    delta = 0;
    for run = 1:numBins
        for i = 1:numBins
            % compute current range size
            if i == 1
                curr = rangeMap(i);
            else
                curr = rangeMap(i)-rangeMap(i-1);
            end
            % add difference to successor if assigned range exceeds cardinality of current bin
            if curr-gamma(i) > 0 % exceeding!
                delta = curr - gamma(i) %functools.reduce(lambda x,y: x*y, self._Gamma[i])
                if i ~= numBins
                    rangeMap(i) = rangeMap(i) - delta;
                else  % shift upper bounds of all predecessors => bin 0 becomes enlarged
                    rangeMap = horzcat(arrayfun(@(item) item+delta, rangeMap(1:end-1)), rangeMap(end));
                end
            end
        end
    end
    *
    * */
   for (unsigned int run = 0; run < _numberOfBuckets; ++run){
       for (unsigned int i = 0; i<_numberOfBuckets; ++i){
           // compute current range size
           if (i == 0)
               curr = rangeMap.at(i);
           else
               curr = rangeMap.at(i)-rangeMap.at(i-1);
           // increment successor's range if assigned range exceeds cardinality of current bin
           delta = curr - _buckets.at(i).length();
           if (delta > 0){ // exceeding!
               if (i < _numberOfBuckets-1){ // correct upper bin edge and continue
                   rangeMap.at(i) = rangeMap.at(i) - delta;
                   cout << "\ncorrected upper limit for bin["<<i<<"] = " << rangeMap.at(i) << endl;
               }
               else{  // shift upper bounds of all predecessors => bin 0 becomes enlarged
                   //rangeMap = horzcat(arrayfun(@(item) item+1, rangeMap(1:end-1)), rangeMap(end));
            	   // TODO: test whether last right bin edge does not exceed _sample
            	   for (unsigned int j = 0; j < _numberOfBuckets-1; ++j)
            		   rangeMap.at(j) += delta;
               }
           }
       }
   }
	return bucketID;
}
/*
 * Adjust _tupleID to lower bin edge by subtracting lowest _tupleID for this bucket.
 */
template<typename T>
inline I64u JointPrFunction<T>::normalizeTupleID(I64u tupleID, size_t bucketID) const
{
	Decimal cs = 0;
	for (size_t i = 0; i < bucketID; ++ i)
		cs += _bucketProbabilities[i];
	I64u tupleID_left = ceil(cs*_sampleSize);
	tupleID -= tupleID_left;
	if (tupleID < 0) throw LogicalException("Index error for normalization of tupleID in JointPrFunction::normalizeTupleID()");
	return tupleID;
}

/*
 * Shuffle _tupleID <- [0; _sampleSize]
 */
template<typename T>
inline I64u JointPrFunction<T>::permuteSampleID(I64u tupleID) const
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
	I64u cardinality = (I64u) _buckets.at(bucketID).length();
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
			gamma *= _buckets.at(bucketID).length(j);
		compositeID[i] = rem/gamma; // div rounds to floor
		rem = rem % gamma;
	}
	compositeID[_dim-1] = rem;
	// map indices to attribute values, use Interval

	// create new Tuple/Triple instance
	// if dim = 2  T = MyriadTuple(ValueType1 val1, ValueType2 val2)
	// TODO: how to convert I64u values into Domain values?
	T t(compositeID[0], compositeID[1]);
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
double JointPrFunction<T>::cdf(const I64u _binID) const
{
	if (0 >_binID || _binID >= _numberOfBuckets)
		throw ConfigException("Unexpected bucketID: out of range");
	return this->_cumulativeProbabilities.at(_binID);
}

template<typename T>
void JointPrFunction<T>::initialize(const string& path)
{
	cout << "init1 ..." << endl;
    initialize(Path(path));
}

template<typename T>
void JointPrFunction<T>::initialize(const Path& path)
{
	cout << "init2 ..." << endl;
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
	cout << "init4 ..." << endl;
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
    	cout << "current state: " << currentState << endl;
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

        	cout << "set _activeDomain" << endl;
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
        //cout << "current line = " << currentLine << endl;

        if (currentState == NOE)
        {
        	// cout << "entering state NOE" << endl;
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
        	//cout << "entered NOB stmt" << endl;
	        if (!in.good() || !headerLine2Format.match(currentLine, 0, posVec))
	        {
	        	//cout << "headerLine2Format does not match" << endl;
		        throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofbins = ' + x", currentLineNumber, currentLine));
	        }
	        //cout << "NOB1" << endl;
	        I32 numberOfBuckets = atoi(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());
	        //cout << "numberOfBuckets = " << numberOfBuckets << endl;

	        if (numberOfBuckets <= 0 && numberOfBuckets > 65536)
	        {
	        	//cout << "throws NOB exception" << endl;
		        throw DataException("Invalid number of buckets '" + toString(numberOfBuckets) +  "'");
	        }

	        /* TODO: initialize bin, bin probabilities */
	        _numberOfBuckets = numberOfBuckets;
	        // TODO: more sophisticated vector initialization?
	        for (I32u i = 0; i < _numberOfBuckets; ++i) _cumulativeProbabilities.push_back(0.0);
	        currentState = NPR;
	        //cout << "NOB2" << endl;
        }
        else if (currentState == NPR)
        {
        	//cout << "entered state NPR" << endl;
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
        	//cout << "entered state VLN" << endl;
	        if (!in.good() || !valueLineFormat.match(currentLine, 0, posVec))
	        {
		        throw DataException(format("line %hu: Bad value probability line `%s`, should be: 'p(X) = ' + p_x + ' for X = {' + x + ' }'", currentLineNumber, currentLine));
	        }


	        Decimal probability = fromString<Decimal>(currentLine.substr(posVec[1].offset, posVec[1].length));
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
        	cout << "entered state BLN" << endl;
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
	        cout << "BLN 1" << endl;

	        T min(min1, min2);
	        T max(max1, max2);
	        IntervalTuple<T> interval(min, max); // set new interval as a set of lower and upper bin edges
	        cout << "BLN 2" << endl;

	        _buckets.push_back(interval); // .at(currentItemIndex).set(min, max);
	        cout << "BLN 3" << endl;
	        _bucketProbabilities.push_back(probability);
	        _dim = 2; // TODO: more generic version - extract from regex
	        _bucketProbability += probability;
	        cout << "BLN 4" << endl;

	        _cumulativeProbabilities.at(currentItemIndex+_numberOfValues) = _valueProbability + _bucketProbability;
	        cout << "_cdfProb [" << currentItemIndex << "] = " << _cumulativeProbabilities.at(currentItemIndex) << ", valueProb = " << _valueProbability << ", buckProb = " << _bucketProbability<< endl;
	        cout << "BLN 5" << endl;
	        currentItemIndex++;

	        if (currentItemIndex >= _numberOfBuckets)
	        {
		        currentState = FIN;
		        currentItemIndex = 0;
	        }
        }
        //cout << "end while-loop" << endl;
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
    cout << "Leaving initialize" << endl;
}

template<typename T>
inline size_t JointPrFunction<T>::numberOfBuckets() const
{
    return _numberOfBuckets;
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
