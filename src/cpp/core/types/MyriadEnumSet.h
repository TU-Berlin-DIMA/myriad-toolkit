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

#ifndef MYRIADENUMSET_H_
#define MYRIADENUMSET_H_

#include <Poco/Any.h>
#include <Poco/Path.h>
#include <Poco/RefCountedObject.h>

#include <iostream>
#include <map>
#include <vector>

using namespace std;
using namespace Poco;

namespace Myriad {
/**
 * @addtogroup core_types
 * @{*/

/**
 * A lightweight implementation of an Enum type.
 *
 * This is a wrapper around a vector of strings that are used as an enum type,
 * together with a corresponding initialization logic that reads an the enum
 * values from the corresponding specification format.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
class MyriadEnumSet : public Poco::RefCountedObject
{
public:

    /**
     * Default constructor.
     *
     * Merely creates a new enum object, and does not execute any
     * initialization routines.
     */
    MyriadEnumSet()
    {
    }

    /**
     * Anonymous file path initialization constructor.
     *
     * Loads the specification for this enum from the file given by the
     * \p path parameter.
     *
     * @param path The location of the enum specification file.
     */
    MyriadEnumSet(const string& path) :
        _name("")
    {
        initialize(path);
    }

    /**
     * Anonymous stream initialization constructor.
     *
     * Loads the specification for this enum from the input stream given by the
     * \p in parameter.
     *
     * @param in Input stream containing the enum specification.
     */
    MyriadEnumSet(istream& in) :
        _name("")
    {
        initialize(in);
    }

    /**
     * Named file path initialization constructor.
     *
     * Loads the configuration for this probability function from the file
     * given by the \p path parameter.
     *
     * @param name The name of this probability function instance.
     * @param path The location of the function configuration file.
     */
    MyriadEnumSet(const string& name, const string& path) :
        _name(name)
    {
        initialize(path);
    }

    /**
     * Named stream initialization parameter.
     *
     * Loads the configuration for this probability function from the input
     * stream given by the \p in parameter.
     *
     * @param name The name of this probability function instance.
     * @param in Input stream containing the function configuration.
     */
    MyriadEnumSet(const string& name, istream& in) :
        _name(name)
    {
        initialize(in);
    }

    /**
     * Anonymous ObjectBuilder constructor.
     *
     * Loads the configuration for this enum from the input stream given by the
     * <tt>params['path']</tt> in parameter.
     *
     * @param params An array containing the required function parameters.
     */
    MyriadEnumSet(map<string, Any>& params) :
        _name(AnyCast<string>(params["name"]))
    {
        initialize(AnyCast<string>(params["path"]));
    }

    /**
     * Initialization routine.
     *
     * Initializes the enum set with the values specified in the file
     * located at the given \p path.
     *
     * @param path The location of the enum specification file.
     */
    void initialize(const string& path);

    /**
     * Initialization routine.
     *
     * Initializes the enum set with the values specified in the file
     * located at the given \p path.
     *
     * @param path The location of the enum specification file.
     */
    void initialize(const Path& path);

    /**
     * Initialization routine.
     *
     * Initializes the enum set with the values specified by the input stream
     * given by the \p in parameter.
     *
     * @param in Input stream containing the enum specification.
     */
    void initialize(istream& in);

    /**
     * Initialization routine.
     *
     * Initializes the enum set with the values specified by the input stream
     * given by the \p in parameter. When reading from the \p in stream, uses
     * the \p currentLineNumber parameter to track the current line number.
     *
     * @param currentLineNumber A reference to the current line number.
     * @param in Input stream containing the enum specification.
     */
    void initialize(istream& in, UInt16& currentLineNumber);

    /**
     * Return the name of this enum set.
     *
     * @return The name of this enum set.
     */
    const string& name() const
    {
        return _name;
    }

    /**
     * Retrieves the enumerated set identified by the given \p key.
     *
     * @return a const reference to the enumerated set identified by \p key
     */
    const vector<string>& values() const
    {
        return _values;
    }

private:

    /**
     * Protected destructor (prohibit static allocation).
     */
    virtual ~MyriadEnumSet()
    {
        reset();
    }

    void reset();

    /**
     * The name of this enum set.
     */
    const string _name;

    /**
     * The indexed set of values for this enum set.
     */
    vector<string> _values;
};

/** @}*/// add to core group
}  // namespace Myriad



#endif /* MYRIADENUMSET_H_ */
