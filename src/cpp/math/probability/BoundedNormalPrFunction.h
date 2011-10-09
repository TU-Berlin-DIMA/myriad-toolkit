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

#ifndef BOUNDEDNORMALPRFUNCTION_H_
#define BOUNDEDNORMALPRFUNCTION_H_

#include "math/probability/NormalPrFunction.h"

using namespace Poco;

namespace Myriad {

class BoundedNormalPrFunction: public NormalPrFunction
{
public:

	BoundedNormalPrFunction(Decimal mean, Decimal stddev, Decimal xMin, Decimal xMax) :
		NormalPrFunction(mean, stddev), _xMin(xMin), _xMax(xMax)
	{
		initialize();
	}

	BoundedNormalPrFunction(const string& name, Decimal mean, Decimal stddev, Decimal xMin, Decimal xMax) :
		NormalPrFunction(name, mean, stddev), _xMin(xMin), _xMax(xMax)
	{
		initialize();
	}

	BoundedNormalPrFunction(map<string, Any>& params) :
		NormalPrFunction(params)
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);

		initialize();
	}

	BoundedNormalPrFunction(const string& name, map<string, Any>& params) :
		NormalPrFunction(name, params)
	{
		_xMin = AnyCast<Decimal>(params["xMin"]);
		_xMax = AnyCast<Decimal>(params["xMax"]);

		initialize();
	}

	Decimal sample(Decimal random) const;

private:

	/**
	 * Common initialization logic.
	 */
	void initialize()
	{
		_yMin = cdf(_xMin);
		_yMax = cdf(_xMax);
		_yFactor = _yMax - _yMin;
	}

	Decimal _xMin;
	Decimal _xMax;
	Decimal _yMin;
	Decimal _yMax;
	Decimal _yFactor;
};

inline Decimal BoundedNormalPrFunction::sample(Decimal random) const
{
	return invcdf(_yMin + _yFactor * random);
}

} // namespace Myriad

#endif /* BOUNDEDNORMALPRFUNCTION_H_ */
