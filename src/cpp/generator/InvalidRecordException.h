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

#ifndef INVALIDRECORDEXCEPTION_H_
#define INVALIDRECORDEXCEPTION_H_

namespace Myriad
{

class InvalidRecordException
{
public:

    InvalidRecordException(I64u currentGenID, I64u maxPeriodSize, I64u currentPeriodSize) :
    	_currentGenID(currentGenID)
    {
    	_nextValidGenID = ((_currentGenID/maxPeriodSize)+1)*maxPeriodSize;
    	_prevValidGenIDMin = _nextValidGenID - maxPeriodSize;
    	_prevValidGenIDMax = _prevValidGenIDMin + currentPeriodSize;
    }

    I64u currentGenID() const
    {
        return _currentGenID;
    }

    I64u nextValidGenID() const
    {
        return _nextValidGenID;
    }

    I64u prevValidGenIDMin() const
    {
        return _prevValidGenIDMin;
    }

    I64u prevValidGenIDMax() const
    {
        return _prevValidGenIDMax;
    }

    I64u prevValidGenIDSize() const
    {
    	return _prevValidGenIDMax - _prevValidGenIDMin;
    }

    I64u invalidRangeSize() const
    {
        return _nextValidGenID-_currentGenID;
    }

private:

    I64u _currentGenID;
    I64u _nextValidGenID;
    I64u _prevValidGenIDMin;
    I64u _prevValidGenIDMax;
};

} // namespace Myriad

#endif /* INVALIDRECORDEXCEPTION_H_ */
