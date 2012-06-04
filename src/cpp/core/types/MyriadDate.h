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

#include <iostream>

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

	MyriadDate(const DateTime& dateTime)
	{
		_dateTime.assign(dateTime.year(), dateTime.month(), dateTime.day());
	}

	MyriadDate(const string& date)
	{
		string s(date);
		int tzd;

		DateTimeParser::parse(s.substr(0,10), _dateTime, tzd);
		resetTime();
	}

	MyriadDate(const char* date)
	{
		string s(date);
		int tzd;

		DateTimeParser::parse(s.substr(0,10), _dateTime, tzd);
		resetTime();
	}

	bool operator == (const MyriadDate& myriadDate) const
	{
		return _dateTime == myriadDate._dateTime;
	}

	bool operator != (const MyriadDate& myriadDate) const
	{
		return _dateTime != myriadDate._dateTime;
	}

	bool operator <  (const MyriadDate& myriadDate) const
	{
		return _dateTime < myriadDate._dateTime;
	}

	bool operator <= (const MyriadDate& myriadDate) const
	{
		return _dateTime <= myriadDate._dateTime;
	}

	bool operator >  (const MyriadDate& myriadDate) const
	{
		return _dateTime > myriadDate._dateTime;
	}

	bool operator >= (const MyriadDate& myriadDate) const
	{
		return _dateTime >= myriadDate._dateTime;
	}

	MyriadDate& operator ++(int)
	{
		_dateTime += Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	MyriadDate& operator ++()
	{
		_dateTime += Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	MyriadDate& operator --(int)
	{
		_dateTime -= Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	MyriadDate& operator --()
	{
		_dateTime -= Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	MyriadDate operator +(const Int64& daysSpan) const
	{
		return MyriadDate(_dateTime + Timespan(daysSpan, 0, 0, 0, 0));
	}

	MyriadDate operator -(const Int64& daysSpan) const
	{
		return MyriadDate(_dateTime - Timespan(daysSpan, 0, 0, 0, 0));
	}

	Int64 operator -(const MyriadDate& myriadDate) const
	{
		return (_dateTime - myriadDate._dateTime).days();
	}

	MyriadDate& operator +=(const Timespan& daysSpan)
	{
		_dateTime += Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	MyriadDate& operator -=(const Timespan& daysSpan)
	{
		_dateTime -= Timespan(1, 0, 0, 0, 0);
		return *this;
	}

	friend ostream &operator<<(ostream& stream, const MyriadDate& ob);
	friend istream &operator>>(istream& stream, MyriadDate& ob);

private:

	void resetTime()
	{
		_dateTime.assign(_dateTime.year(), _dateTime.month(), _dateTime.day());
	}

	DateTime _dateTime;
};

inline ostream &operator<<(ostream& stream, const MyriadDate& ob)
{
	stream << Poco::DateTimeFormatter::format(ob._dateTime, "%Y-%m-%d");
	return stream;
}

inline istream &operator>>(istream& stream, MyriadDate& ob)
{
	int tzd;
	char line[10];
	stream.read(line, 10);
	DateTimeParser::parse(string(line), ob._dateTime, tzd);
	return stream;
}

} // namespace Myriad

#endif /* MYRIADDATE_H_ */
