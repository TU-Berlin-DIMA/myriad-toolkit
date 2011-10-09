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

#ifndef UTIL_H_
#define UTIL_H_

#include "core/exceptions.h"
#include "core/types.h"

#include <Poco/Format.h>

#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cmath>

using namespace Poco;
using namespace std;

namespace Myriad {

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// function declarations
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

/**
 * Calculate x * y % N
 *
 * @param x
 * @param y
 * @param N
 * @param InvN
 * @return
 */
I64 modmult(const I64 x, const I64 y, const I64 N, const Decimal InvN);

/**
 * Calculate (x ^ e % n) fast.
 *
 * @param x
 * @param e
 * @param N
 * @return
 */
I64u modexp(I64u x, I64u e, I64u N);

/**
 * Run a Miller-Rabin primality with loop length k on the input n.
 *
 * @param n
 * @param k
 * @return
 */
bool isPrime(I64u N, const I16u k);

/**
 * Return the smallest prime greater or equal to the lower bound l.
 *
 * @param l
 * @return
 */
I64u nextPrime(const I64u l);

/**
 * Compute the integer power x^e.
 *
 * @param x
 * @param e
 * @return x^e
 */
I64u power(I32u x, I16u e);

/**
 * Compute the nearest power of 2 of x.
 *
 * @param x
 * @return
 */
I64u nearestPow2(I64u x);

/**
 * Compute the log2 of an exact power of 2.
 *
 * @param x
 * @return
 */
I16u log2exact(I64u x);

// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
// inline function definitions
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~

inline I64 modmult(const I64 x, const I64 y, const I64 N, const Decimal InvN)
{
	// classical trick to bypass the 64-bit limitation, when x*y does not fit into the I64u type.
	// works whenever x*y/N is less than 2^52 (double type maximal precision)
	I64 q = (I64) (InvN * (Decimal) x * (Decimal) y);
	I64 r = x * y - q * N;
	return r < 0 ? r + N : r % N;
}


inline I64u modexp(I64u x, I64u e, I64u N)
{
	I64u y = 1ULL;
	I64u a = x;
	Decimal InvN = 1.0/(Decimal) N;

	while (e != 0)
	{
		if (e & 1ULL)
		{
//			y = (y * a) % N;
			y = modmult(y, a, N, InvN);
		}

//		a = (a * a) % N;
		a = modmult(a, a, N, InvN);

		e >>= 1;
	}

	return y;
}

inline bool isPrime(I64u N, const I16u k = 9)
{
	if (N > 2251799813685248ULL)
	{
		throw IntegerOverflowException(format("Integer %Lu is too big for modular arithmetic operations", N));
	}


	// write n-1 as 2^s*d with d odd
	I64u s = 0;
	I64u d = N - 1;
	Decimal Invn = 1.0/(Decimal) N;

	if (N < 2)
	{
		return false;
	}

	while (d % 2 == 0)
	{
		d >>= 1;
		s++;
	}

	for (int i = 0; i < k; i++)
	{
		// pick a randomly in the range [2, n-2]
		I64u a = (rand() % (N - 2)) + 2;

		I64u x = modexp(a, d, N);

		if (x == 1 || x == N - 1)
		{
			continue;
		}

		bool continueFlag = false;
		for (unsigned long r = 1; r < s; r++)
		{
//			x = (x * x) % n;
			x = modmult(x, x, N, Invn);

			if (x == 1)
			{
				return false;
			}

			if (x == N - 1)
			{
				continueFlag = true;
				break;
			}
		}

		if (continueFlag)
		{
			continue;
		}
		else
		{
			return false;
		}
	}

	return true;
}

inline I64u nextPrime(const I64u l)
{
	I64u N = l;

	while (true)
	{
		while (N % 2 == 0 || N % 5 == 0) // 2|n or 5|n
		{
			N++;
		}


		if (isPrime(N))
		{
			return N;
		}

		N += 2;
	}
}

inline I64u power(I32u x, I16u e)
{
	if (x == 0)
	{
		return 0;
	}
	else if (x == 1)
	{
		return 1;
	}
	else if (x == 2)
	{
		return 1 << e;
	}
	else
	{

		I64u r = 1;
		for (I16u i = 0; i < e; i++)
		{
			r *= x;
		}
		return r;
	}
}

inline I64u nearestPow2(I64u n)
{
	I64u u, l;

	if ((n & (1ULL << 63)) != 0) // most significant bit is set
	{
		return (1ULL << 63);
	}

	// compute nearest power of 2 higher than n
	u = n-1;
	// set all bits left of the first '1' bit to '1'
	u |= u >> 1;
	u |= u >> 2;
	u |= u >> 4;
	u |= u >> 8;
	u |= u >> 16;
	u |= u >> 32;
	u++;

	// compute nearest power of 2 smaller than n
	l = u >> 1;

	// return l or u depending on which is closer to n
	return (n-l) < (u-n) ? l : u;
}

inline I16u log2exact(I64u x)
{
	I64u a = 1;

	for (I16u i = 0; i < 64; i++)
	{
		if (a == x)
		{
			return i;
		}
		a <<= 1;
	}

	cerr << x << " is not a power of two" << endl;

	throw exception();
}

} // namespace Myriad

#endif /* UTIL_H_ */
