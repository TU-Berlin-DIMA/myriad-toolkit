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
 * @author: Alexander Alexandrov <alexander.s.alexandrov@campus.tu-berlin.de>
 */

#ifndef DISCRETEDISTRIBUTION_H_
#define DISCRETEDISTRIBUTION_H_

#include "core/types.h"
#include "math/Function.h"
#include "math/VectorMap.h"

namespace Myriad {

template<class ProbabilityType> class DiscreteDistribution: public UnaryFunction<I64u, I64u>
{
public:

	DiscreteDistribution(const ProbabilityType& probability) :
		UnaryFunction<I64u, I64u> ("discrete distribution over " + probability.name()), _probability(probability)
	{
		Interval<Decimal> threshold = _probability.threshold(0.0005);
		_xMin = threshold.min();
		_xMax = threshold.max();
	}

	DiscreteDistribution(const ProbabilityType& probability, I64u xSize, I64u yTotal, I64u yMin = 0) :
		UnaryFunction<I64u, I64u> ("discrete distribution over " + probability.name()), _probability(probability), _xSize(xSize), _yTotal(yTotal), _yMin(yMin)
	{
		Interval<Decimal> threshold = _probability.threshold(0.0005);
		_xMin = threshold.min();
		_xMax = threshold.max();

		initialize(xSize, yTotal, yMin);
	}

	void initialize(I64u xSize, I64u yTotal, I64u yMin = 0);

	I64u operator()(const I64u x) const;

	I64u pdf(const I64u x) const;

	I64u cdf(const I64u x) const;

	VectorMap<I64u>* pdfMap(const string& name) const;

	VectorMap<I64u>* cdfMap(const string& name) const;

private:

	I64u at(const I64u x) const;

	// the backing probability function
	const ProbabilityType& _probability;

	// base parameters
	I64u _xSize;
	I64u _yTotal;
	I64u _yMin;

	// supporting parameters
	Decimal _xMin;
	Decimal _xMax;
	Decimal _step;
	Decimal _scale;
};

template<class ProbabilityType> void DiscreteDistribution<ProbabilityType>::initialize(I64u xSize, I64u yTotal, I64u yMin)
{
	_xSize = xSize;
	_yTotal = yTotal;
	_yMin = yMin;

	_step = (_xMax - _xMin) / static_cast<Decimal> (_xSize);
	_scale = (_yTotal - _xSize * _yMin) / (_probability.cdf(_xMax) - _probability.cdf(_xMin));
}

template<class ProbabilityType> inline I64u DiscreteDistribution<ProbabilityType>::operator()(const I64u x) const
{
	return _yMin + at(x + 1) - at(x);
}

template<class ProbabilityType> I64u DiscreteDistribution<ProbabilityType>::pdf(const I64u x) const
{
	return _yMin + at(x + 1) - at(x);
}

template<class ProbabilityType> I64u DiscreteDistribution<ProbabilityType>::cdf(const I64u x) const
{
	return _yMin * (x+1) + at(x+1) - at(0);
}

template<class ProbabilityType> inline I64u DiscreteDistribution<ProbabilityType>::at(const I64u x) const
{
	return static_cast<I64u> (_scale * _probability.cdf(_xMin + x * _step) + 0.5);
}

template<class ProbabilityType> VectorMap<I64u>* DiscreteDistribution<ProbabilityType>::pdfMap(const string& name) const
{
	VectorMap<I64u>* map = new VectorMap<I64u>(name, _xSize);
	for (I32u x = 0; x < _xSize; x++)
	{
		map->define(x, this->pdf(x));
	}

	return map;
}

template<class ProbabilityType> VectorMap<I64u>* DiscreteDistribution<ProbabilityType>::cdfMap(const string& name) const
{
	VectorMap<I64u>* map = new VectorMap<I64u>(name, _xSize);
	for (I32u x = 0; x < _xSize; x++)
	{
		map->define(x, this->cdf(x));
	}

	return map;
}

} // namespace Myriad

#endif /* DISCRETEDISTRIBUTION_H_ */
