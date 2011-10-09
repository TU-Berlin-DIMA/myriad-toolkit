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

#ifndef PERMUTATOR_H_
#define PERMUTATOR_H_

class Permutator
{
public:
	Permutator(unsigned long g, unsigned long N) : g(g), N(N)
	{
	}

	~Permutator()
	{
	}

	unsigned long exp(unsigned long m)
	{
		return modexp(m);
	}

private:
	/**
	 * Calculate (pow(_min, _max) % n) fast
	 *
	 * @return
	 */
	unsigned long modexp(unsigned long m)
	{
		unsigned long y = 1;
		unsigned long a = g;

		while (m != 0)
		{
			/* Pour chaque 1 de _max, on accumule dans y. */
			if (m & 1)
			{
				y = (y * a) % N;
			}

			/* lvation de _min au carr pour chaque bit de _max. */
			a = (a * a) % N;

			/* On se prpare pour le prochain bit de _max. */
			m >>= 1;
		}
		return y;
	}

	bool isPrime(unsigned long n)
	{
		unsigned short k = 9; // run k-iterations of the Miller-Rabin test

		// write n-1 as 2^s*d with d odd
		unsigned long s = 0;
		unsigned long d = n-1;

		while(d % 2 == 0)
		{
			d >>= 1;
			s++;
		}
	}

	unsigned int g;
	unsigned int N;
};

#endif /* PERMUTATOR_H_ */
