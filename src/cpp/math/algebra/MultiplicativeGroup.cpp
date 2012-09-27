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

#include "math/algebra/MultiplicativeGroup.h"

namespace Myriad {
/**
 * @addtogroup math_algebra
 * @{*/

/**
 * Initialize the generator elements for some cardinalities.
 */
MultiplicativeGroup::GeneratorMap initGenerators()
{
    MultiplicativeGroup::GeneratorMap tmp;

    // precomputed values for sf = 304
    tmp[7296001ULL] = 43ULL;
    tmp[36480023ULL] = 45ULL;
    tmp[182400013ULL] = 43ULL;
    tmp[72960007ULL] = 43ULL;
    tmp[80256013ULL] = 45ULL;
    tmp[729600007ULL] = 43ULL;
    tmp[2918400029ULL] = 42ULL;
    // precomputed values for sf = 352
    tmp[8448007ULL] = 48ULL;
    tmp[42240013ULL] = 42ULL;
    tmp[211200001ULL] = 46ULL;
    tmp[84480001ULL] = 43ULL;
    tmp[92928013ULL] = 44ULL;
    tmp[844800013ULL] = 45ULL;
    tmp[3379200017ULL] = 42ULL;
    // precomputed values for sf = 368
    tmp[8832001ULL]     = 55ULL;
    tmp[44160007ULL]    = 42ULL;
    tmp[220800001ULL]   = 57ULL;
    tmp[88320007ULL]    = 48ULL;
    tmp[97152031ULL]    = 44ULL;
    tmp[883200001ULL]   = 42ULL;
    tmp[3532800001ULL]  = 56ULL;
    // precomputed values for sf = 760
    tmp[18240007ULL] = 43ULL;
    tmp[91200007ULL] = 45ULL;
    tmp[456000023ULL] = 45ULL;
    tmp[182400013ULL] = 43ULL;
    tmp[200640023ULL] = 44ULL;
    tmp[1824000001ULL] = 43ULL;
    tmp[7296000011ULL] = 42ULL;
    // precomputed values for sf = 880
    tmp[21120017ULL] = 42ULL;
    tmp[105600007ULL] = 45ULL;
    tmp[528000017ULL] = 43ULL;
    tmp[211200001ULL] = 46ULL;
    tmp[232320001ULL] = 47ULL;
    tmp[2112000017ULL] = 42ULL;
    tmp[8448000037ULL] = 42ULL;
    // precomputed values for sf = 920
    tmp[22080053ULL]    = 43ULL;
    tmp[110400001ULL]   = 42ULL;
    tmp[552000011ULL]   = 43ULL;
    tmp[220800001ULL]   = 57ULL;
    tmp[242880031ULL]   = 48ULL;
    tmp[2208000007ULL]  = 42ULL;
    tmp[8832000017ULL]  = 45ULL;
    // precomputed values for sf = 1520
    tmp[36480023ULL] = 45ULL;
    tmp[182400013ULL] = 43ULL;
    tmp[912000017ULL] = 45ULL;
    tmp[364800043ULL] = 43ULL;
    tmp[401280023ULL] = 43ULL;
    tmp[3648000007ULL] = 44ULL;
    tmp[14592000037ULL] = 43ULL;
    // precomputed values for sf = 1760
    tmp[42240013ULL] = 42ULL;
    tmp[211200001ULL] = 46ULL;
    tmp[1056000007ULL] = 42ULL;
    tmp[422400047ULL] = 44ULL;
    tmp[464640013ULL] = 45ULL;
    tmp[4224000019ULL] = 42ULL;
    tmp[16896000047ULL] = 44ULL;
    // precomputed values for sf = 1840
    tmp[44160007ULL]    = 42ULL;
    tmp[220800001ULL]   = 57ULL;
    tmp[1104000013ULL]  = 43ULL;
    tmp[441600013ULL]   = 42ULL;
    tmp[485760007ULL]   = 42ULL;
    tmp[4416000031ULL]  = 44ULL;
    tmp[17664000041ULL] = 42ULL;

    return tmp;
}

MultiplicativeGroup::GeneratorMap MultiplicativeGroup::GENERATORS(initGenerators());

void MultiplicativeGroup::configure(I64u cardinality)
{
    _logger.information(format("Configuring generator for cardinality %Lu", cardinality));

    N = nextPrime(cardinality);

    _logger.information(format("Closest prime for cardinality %Lu is %Lu", cardinality, N));

    if (GENERATORS.find(N) == GENERATORS.end()) // generator for this cyclic group is not in cache
    {
        I64u a, o, x;

        a = 42; // start with the ultimate answer - 42!!!

        _logger.information(format("Running naive search for a generator of (Z/%LuZ)", N));

        do
        {
	        a++;

	        x = a;
	        o = 1;

	        do
	        {
		        x = a * x % N;
		        o++;
	        }
	        while (x != 1); // multiply until you come to 1
        }
        while (o != N - 1); // continue seeking until the order of the element equals the order of the group

        _logger.information(format("Storing naive generator search result for (Z/%LuZ) - %Lu in cache", N, a));

        GENERATORS[N] = a; // store the answer in the cache
    }

    g = GENERATORS[N];

    _logger.information(format("Generator of the unit group of (Z/%LuZ) is set to %Lu", N, g));
}

/** @}*/// add to math_algebra group
} // namespace Myriad
