/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef TESTSOCK_H
#define TESTSOCK_H

#define DATASTR "This is a test"
#define STRLEN 8092

/* This is a hack.  We can't return APR_TIMEOUT from sockchild, because
 * Unix OSes only return the least significant 8 bits of the return code,
 * which means that instead of receiving 70007, testsock gets 119.  But,
 * we also don't want to return -1, because we use that value for general
 * errors from sockchild.  So, we define 1 to mean that the read/write
 * operation timed out.  This means that we can't write a test that tries
 * to send a single character between ends of the socket.
 */
#define SOCKET_TIMEOUT 1

#endif

