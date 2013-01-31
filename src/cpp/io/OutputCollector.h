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

#ifndef OUTPUTCOLLECTOR_H_
#define OUTPUTCOLLECTOR_H_

#include "io/LocalFileOutputCollector.h"

namespace Myriad {
/**
 * @addtogroup io
 * @{*/

/**
 * A generic traits structure for I/O related type information.
 *
 * @author: Alexander Alexandrov <alexander.alexandrov@tu-berlin.de>
 */
template<class RecordType> struct OutputTraits
{
    /**
     * An alias to the output collector implementation to be used.
     */
    typedef LocalFileOutputCollector<RecordType> CollectorType;
};

/** @}*/// add to io group
} // namespace Myriad

#endif /* OUTPUTCOLLECTOR_H_ */
