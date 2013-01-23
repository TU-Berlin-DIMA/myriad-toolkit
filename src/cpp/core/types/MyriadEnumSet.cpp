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

#include <fstream>

using namespace Poco;

namespace Myriad {

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
    string currentLine;

    // read first line
    getline(in, currentLine);

    if (!in.good() || currentLine.substr(0, 17) != "@numberofvalues =")
    {
        throw DataException(format("line %hu: Bad header line `%s`, should be: '@numberofvalues = ' + x", currentLineNumber, currentLine));
    }

    I32 numberOfEntries = atoi(currentLine.substr(17).c_str());

    _values.resize(numberOfEntries);

    for (I16u i = 0; i < numberOfEntries; i++)
    {
        if (!in.good())
        {
            throw DataException("Bad line for bin #" + toString(i));
        }

        getline(in, currentLine);

        string::size_type t = currentLine.find_first_of('\t');
        string::size_type c = currentLine.find_first_of('#');
        string value;

        if (t == string::npos)
        {
            throw DataException("Bad line for bin #" + toString(i) + ", expected format '{i}\t{value}[# {comment}]");
        }

        if (c != string::npos)
        {
            value = currentLine.substr(t+1, c-t-1);
            trim(value);
        }
        else
        {
            value = currentLine.substr(t+1);
            trim(value);
        }

        _values[i] = value;
    }
}

void MyriadEnumSet::reset()
{
    // do nothing
}

} // namespace Myriad
