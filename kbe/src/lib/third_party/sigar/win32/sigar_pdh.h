/*
 * Copyright (c) 2004, 2006 Hyperic, Inc.
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
 */

#ifndef SIGAR_PDH_H
#define SIGAR_PDH_H

/* performance data helpers */

#define PdhFirstObject(block) \
    ((PERF_OBJECT_TYPE *)((BYTE *) block + block->HeaderLength))

#define PdhNextObject(object) \
    ((PERF_OBJECT_TYPE *)((BYTE *) object + object->TotalByteLength))

#define PdhFirstCounter(object) \
    ((PERF_COUNTER_DEFINITION *)((BYTE *) object + object->HeaderLength))

#define PdhNextCounter(counter) \
    ((PERF_COUNTER_DEFINITION *)((BYTE *) counter + counter->ByteLength))

#define PdhGetCounterBlock(inst) \
    ((PERF_COUNTER_BLOCK *)((BYTE *) inst + inst->ByteLength))

#define PdhFirstInstance(object) \
    ((PERF_INSTANCE_DEFINITION *)((BYTE *) object + object->DefinitionLength))

#define PdhNextInstance(inst) \
    ((PERF_INSTANCE_DEFINITION *)((BYTE *)inst + inst->ByteLength + \
                                  PdhGetCounterBlock(inst)->ByteLength))

#define PdhInstanceName(inst) \
    ((wchar_t *)((BYTE *)inst + inst->NameOffset))

#endif /* SIGAR_PDH_H */
