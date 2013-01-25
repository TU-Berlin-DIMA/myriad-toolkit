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

#include "core/exceptions.h"
#include "core/types.h"
#include "core/types/MyriadEnumSet.h"

#include <Poco/File.h>
#include <Poco/Format.h>
#include <Poco/NumberParser.h>
#include <Poco/String.h>

#include <fstream>

using namespace Poco;

namespace Myriad {

////////////////////////////////////////////////////////////////////////////////
/// @name Static Template Members
////////////////////////////////////////////////////////////////////////////////
//@{

RegularExpression MyriadEnumSet::headerLine1Format("\\s*@numberofvalues\\s*=\\s*([+]?[0-9]+)\\s*(#(.+))?");
RegularExpression MyriadEnumSet::valueLineFormat("\\s*[0-9]*\\s*\\.\\.\\.+\\s*\"(.*)\"\\s*(#(.+))?");

//@}

////////////////////////////////////////////////////////////////////////////////
/// @name Member Functions
////////////////////////////////////////////////////////////////////////////////
//@{

void MyriadEnumSet::initialize(const string& path)
{
    initialize(Path(path));
}

void MyriadEnumSet::initialize(const Path& path)
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

void MyriadEnumSet::initialize(istream& in)
{
    I16u currentLineNumber = 1;
    initialize(in, currentLineNumber);
}

void MyriadEnumSet::initialize(istream& in, I16u& currentLineNumber)
{
    enum READ_STATE { NOV, VLN, END };

    // reset old state
    reset();

    // reader variables
    READ_STATE currentState = NOV; // current reader machine state
    string currentLine; // the current line
    I16u currentItemIndex = 0; // current item index
    RegularExpression::MatchVec posVec; // a posVec for all regex matches

    // reader finite state machine
    while (currentState != END)
    {
        // read next line
        currentLine = "";
        getline(in, currentLine);

        // trim whitespace and unescape quotes
        trimInPlace(currentLine);

        // check if this line is empty or contains a single comment
        if (currentLine.empty() || currentLine.at(0) == '#')
        {
            if (!in.good()) // break on end of stream
            {
                if (currentState == VLN && currentItemIndex < _numberOfValues)
                {
                    throw DataException(format("line %hu: Bad enum value line, should be: '%hu ........... $value' (not enough items specified?)", currentLineNumber, currentItemIndex));
                }
                else
                {
                    currentState = END;
                }
            }

            currentLineNumber++;
            continue; // skip this line
        }

        if (currentState == NOV)
        {
            if (!headerLine1Format.match(currentLine, 0, posVec))
            {
                throw DataException(format("line %hu: Bad header line, should be: '@numberofvalues = $N'", currentLineNumber));
            }

            I64 numberOfValues = NumberParser::parse64(currentLine.substr(posVec[1].offset, posVec[1].length).c_str());

            if (numberOfValues <= 0 && static_cast<size_t>(numberOfValues) > numeric_limits<size_t>::max())
            {
                throw DataException("Invalid number of values '" + toString(numberOfValues) +  "'");
            }

            _numberOfValues = numberOfValues;
            _values.resize(_numberOfValues);

            currentState = VLN;
        }
        else if (currentState == VLN)
        {
            if (!valueLineFormat.match(currentLine, 0, posVec))
            {
                throw DataException(format("line %hu: Bad enum value line, should be: '%hu ........... $value' (missing new line at the end of file?)", currentLineNumber, currentItemIndex));
            }

            String value = currentLine.substr(posVec[1].offset, posVec[1].length);
            replaceInPlace(value, "\\\"", "\"");
            replaceInPlace(value, "\\n", "\n");

            _values[currentItemIndex] = value;
            currentItemIndex++;

            if (currentItemIndex >= _numberOfValues)
            {
                currentState = END;
                currentItemIndex = 0;
            }
        }
    }

    // protect against unexpected reader state
    if (currentState != END)
    {
        throw RuntimeException("Unexpected state in CombinedPrFunction reader at line " + currentLineNumber);
    }
}

void MyriadEnumSet::reset()
{
    // do nothing
}

//@}

} // namespace Myriad
