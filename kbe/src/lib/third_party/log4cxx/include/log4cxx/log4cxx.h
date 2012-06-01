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

#ifndef LOG4CXX_LOG4CXX_H
#define LOG4CXX_LOG4CXX_H

/* GENERATED FILE WARNING!  DO NOT EDIT log4cxx.h
 *
 * Edit log4cxx.hw instead
 *
 */

#define LOG4CXX_LOGCHAR_IS_UTF8 0
#if LOG4CXX_LOGCHAR_IS_UTF8
#define LOG4CXX_LOGCHAR_IS_WCHAR 0
#else
#define LOG4CXX_LOGCHAR_IS_WCHAR 1
#endif
#define LOG4CXX_LOGCHAR_IS_UNICHAR 0

#define LOG4CXX_CHAR_API 1
#define LOG4CXX_WCHAR_T_API 1
#define LOG4CXX_UNICHAR_API 0
#define LOG4CXX_CFSTRING_API 0

#if defined(_MSC_VER)
typedef __int64 log4cxx_int64_t;
#if _MSC_VER < 1300
#define LOG4CXX_USE_GLOBAL_SCOPE_TEMPLATE 1
#define LOG4CXX_LOGSTREAM_ADD_NOP 1
#endif
#elif defined(__BORLANDC__)
typedef __int64 log4cxx_int64_t;
#else
typedef long long log4cxx_int64_t;
#endif

typedef log4cxx_int64_t log4cxx_time_t;
typedef int log4cxx_status_t;
typedef unsigned int log4cxx_uint32_t;

//  definitions used when using static library
#if defined(LOG4CXX_STATIC)
#define LOG4CXX_EXPORT
//   definitions used when building DLL
#elif defined(LOG4CXX)
#define LOG4CXX_EXPORT __declspec(dllexport)
#else
//    definitions used when using DLL
#define LOG4CXX_EXPORT __declspec(dllimport)
#endif


//
//   pointer and list definition macros when building DLL using VC
//
#if defined(_MSC_VER) && !defined(LOG4CXX_STATIC) && defined(LOG4CXX)
#define LOG4CXX_PTR_DEF(T) \
template class LOG4CXX_EXPORT log4cxx::helpers::ObjectPtrT<T>; \
typedef log4cxx::helpers::ObjectPtrT<T> T##Ptr
#define LOG4CXX_LIST_DEF(N, T) \
template class LOG4CXX_EXPORT std::allocator<T>; \
template class LOG4CXX_EXPORT std::vector<T>; \
typedef std::vector<T> N
//
//   pointer and list definition macros when linking with DLL using VC
//
#elif defined(_MSC_VER) && !defined(LOG4CXX_STATIC)
#define LOG4CXX_PTR_DEF(T) \
extern template class LOG4CXX_EXPORT log4cxx::helpers::ObjectPtrT<T>; \
typedef log4cxx::helpers::ObjectPtrT<T> T##Ptr
#define LOG4CXX_LIST_DEF(N, T) \
extern template class LOG4CXX_EXPORT std::allocator<T>; \
extern template class LOG4CXX_EXPORT std::vector<T>; \
typedef std::vector<T> N
//
//   pointer and list definition macros for all other cases
//
#else
#define LOG4CXX_PTR_DEF(T) typedef log4cxx::helpers::ObjectPtrT<T> T##Ptr
#define LOG4CXX_LIST_DEF(N, T) typedef std::vector<T> N
#endif


#endif

