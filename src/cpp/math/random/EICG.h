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

#ifndef EICG_H_
#define EICG_H_

#include "RNG.h"

#include <iostream>
#include <Poco/Types.h>

using namespace Poco;
using namespace std;

namespace Myriad {
/**
 * @addtogroup math_random
 * @{*/

/**
 * An extended version of an inversive congruential generator (ICG).
 *
 * The EICG is defined the following map over the \f$Z_p\f$ field:
 *
 * \f[ y_n := inv(a * (n + i) + b) \f]
 */
class EICG : public RNG
{
public:

	/**
	 * Constructor.
	 */
    EICG(const Int32 p, const Int32 a, const Int32 b, const Int32 n, const Int32 s = 0) :
        p(p), a(a), b(b), n(n), s(s)
    {
    }

    /**
     * See RNG::next()
     */
    double next()
    {
        return at(s++);
    }

    /**
     * See RNG::at()
     */
    double at(UInt64 s)
    {
        Int32 x = 0;

        s = s % p;
        if (b == 0)
        {
	        x = inverse(mult(a, s));
        }
        else
        {
	        x = inverse(add(mult(a, s), b));
        }

        return x / static_cast<double> (p);
    }

    /**
     * See RNG::at()
     */
    void skip(UInt64 pos)
	{
        s += pos;
	}

private:

    /**
     * Computes x+y mod p using fast multiplication.
     */
    Int32 add(Int32 x, Int32 y) const;

    /**
     * Computes x*y mod p using fast multiplication.
     */
    Int32 mult(Int32 x, Int32 y) const;

    /**
     * Computes the inverse of z mod p using the extended Euklidean algorithm.
     */
    Int32 inverse(Int32 z) const;

    /**
     * Performes an extended Euklidean algorithm for the pair (a,b). The
     * result is a triple (x,y, g), such that g is GCD(a,b) and (x,y) are
     * minimal with ax + by = g.
     */
    void extendedGCD(Int32 a, Int32 b, Int32& x, Int32& y, Int32& g) const;

    const Int32 p;
    const Int32 a;
    const Int32 b;
    const Int32 n;

    Int32 s;
};

inline Int32 EICG::add(Int32 x, Int32 y) const
{
    Int32 z = x + y;

    // check for overflow
    if (z < 0 || z >= p)
    {
        z -= p;
    }

    return z;
}

inline Int32 EICG::mult(Int32 x, Int32 y) const
{
    Int32 q, r, z;

    q = p / x;
    r = p % x;

    z = x * (y % q) - r * (y / q);
    while (z < 0)
    {
        z += p;
    }

    return z;
}

inline Int32 EICG::inverse(Int32 z) const
{
    if (z == 0)
    {
        return 0;
    }
    else
    {
        Int32 x, y, gcd;
        extendedGCD(z, p, x, y, gcd);

        if (x < 0)
        {
	        x += p;
        }

        return x;
    }
}

inline void EICG::extendedGCD(Int32 a, Int32 b, Int32& x, Int32& y, Int32& g) const
{
    Int32 lastx, lasty, q, t;

    x = 0;
    y = 1;

    lastx = 1;
    lasty = 0;

    while (b != 0)
    {
        q = a / b;

        t = b;
        b = a % b;
        a = t;

        t = x;
        x = lastx - q * x;
        lastx = t;

        t = y;
        y = lasty - q * y;
        lasty = t;
    }

    x = lastx;
    y = lasty;
    g = a;
}

/** @}*/// add to math group
}  // namespace Myriad

#endif /* EICG_H_ */

