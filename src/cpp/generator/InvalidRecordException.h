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

#ifndef INVALIDRECORDEXCEPTION_H_
#define INVALIDRECORDEXCEPTION_H_

namespace Myriad {
/**
 * @addtogroup generator
 * @{*/

/**
 * An exception when a SetterChain operation is performed on illegal record.
 *
 * The exception is thrown by the runtime components when a value, a range or a
 * reference cannot be provided for the given record context because it lies in
 * a bad (e.g. invalid \p genID). This is generally the case when the
 * sampling is nested and we are trying to instantiate a record that does not
 * have an associated parent, i.e. one of the 'black' records from the range
 * reserved for a particular parent.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class InvalidRecordException
{
public:

    /**
     * Constructor.
     *
     * @param currentGenID The ID of the invalid record that triggered the
     *        exception.
     * @param maxPeriodSize The maximal number of valid records in each
     *        cluster.
     * @param currentPeriodSize The number of valid records in the current
     *        cluster.
     */
    InvalidRecordException(I64u currentGenID, I64u maxPeriodSize, I64u currentPeriodSize) :
        _currentGenID(currentGenID)
    {
        _nextValidGenID = ((_currentGenID/maxPeriodSize)+1)*maxPeriodSize;
        _prevValidGenIDMin = _nextValidGenID - maxPeriodSize;
        _prevValidGenIDMax = _prevValidGenIDMin + currentPeriodSize;
    }

    /**
     * Returns the genID of the current (invalid) record.
     */
    I64u currentGenID() const
    {
        return _currentGenID;
    }

    /**
     * Returns the genID of the next valid record.
     */
    I64u nextValidGenID() const
    {
        return _nextValidGenID;
    }

    /**
     * Returns the start genID (inclusive) of valid record subrange of
     * the period that contains the current invalid record.
     */
    I64u prevValidGenIDMin() const
    {
        return _prevValidGenIDMin;
    }

    /**
     * Returns the end genID (exclusive) of valid record subrange of
     * the period that contains the current invalid record.
     */
    I64u prevValidGenIDMax() const
    {
        return _prevValidGenIDMax;
    }

    /**
     * Returns the size of the valid records genID range in the period that
     * contains the current invalid record.
     */
    I64u prevValidGenIDSize() const
    {
        return _prevValidGenIDMax - _prevValidGenIDMin;
    }

    /**
     * Returns the size of the invalid records genID range in the period that
     * contains the current invalid record.
     */
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

/** @}*/// add to generator group
} // namespace Myriad

#endif /* INVALIDRECORDEXCEPTION_H_ */
