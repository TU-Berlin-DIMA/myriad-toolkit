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

#ifndef MYRIADDATE_H_
#define MYRIADDATE_H_

#include <Poco/DateTime.h>
#include <Poco/DateTimeFormat.h>
#include <Poco/DateTimeFormatter.h>
#include <Poco/DateTimeParser.h>

using namespace Poco;
using namespace std;

namespace Myriad
{

class MyriadDate
{
public:

	MyriadDate()
	{
	}

	MyriadDate(const string& date)
	{
		string s(date);
		int tzd;

		DateTimeParser::parse(s, _dateTime, tzd);
	}

	MyriadDate(const char* date)
	{
		string s(date);
		int tzd;

		DateTimeParser::parse(s, _dateTime, tzd);
	}

	MyriadDate operator +(const Int64& daysSpan) const
	{
		return *this;
	}

	MyriadDate operator -(const Int64& daysSpan) const
	{
		return *this;
	}

	Int64 operator -(const MyriadDate& dateTime) const
	{
		return 0;
	}

	MyriadDate& operator +=(const Timespan& daysSpan)
	{
		return *this;
	}

	MyriadDate& operator -=(const Timespan& daysSpan)
	{
		return *this;
	}

	bool operator <(const MyriadDate& o) const
	{
		return _dateTime < o._dateTime;
	}

	friend ostream &operator<<(ostream& stream, const MyriadDate& ob);
	friend istream &operator>>(istream& stream, MyriadDate& ob);

private:

	DateTime _dateTime;
};

inline ostream &operator<<(ostream& stream, const MyriadDate& ob)
{
	stream << Poco::DateTimeFormatter::format(ob._dateTime, Poco::DateTimeFormat::SORTABLE_FORMAT);
	return stream;
}

inline istream &operator>>(istream& stream, MyriadDate& ob)
{
	return stream;
}

} // namespace Myriad

#endif /* MYRIADDATE_H_ */
