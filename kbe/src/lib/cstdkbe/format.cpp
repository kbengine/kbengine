/*
 Formatting library for C++

 Copyright (c) 2012 - 2014, Victor Zverovich
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
    list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
    this list of conditions and the following disclaimer in the documentation
    and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

// Disable useless MSVC warnings.
#undef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#undef _SCL_SECURE_NO_WARNINGS
#define _SCL_SECURE_NO_WARNINGS

#include "format.hpp"

using fmt::LongLong;
using fmt::ULongLong;
using fmt::internal::Arg;

const char fmt::internal::DIGITS[] =
    "0001020304050607080910111213141516171819"
    "2021222324252627282930313233343536373839"
    "4041424344454647484950515253545556575859"
    "6061626364656667686970717273747576777879"
    "8081828384858687888990919293949596979899";

const uint32_t fmt::internal::POWERS_OF_10_32[] = {0, FMT_POWERS_OF_10(1)};
const uint64_t fmt::internal::POWERS_OF_10_64[] = {
  0,
  FMT_POWERS_OF_10(1),
  FMT_POWERS_OF_10(fmt::ULongLong(1000000000)),
  // Multiply several constants instead of using a single long long constant
  // to avoid warnings about C++98 not supporting long long.
  fmt::ULongLong(1000000000) * fmt::ULongLong(1000000000) * 10
};

const fmt::internal::Arg &fmt::internal::FormatterBase::next_arg() {
  if (next_arg_index_ < 0) {
    if (!error_)
      error_ = "cannot switch from manual to automatic argument indexing";
    return DUMMY_ARG;
  }
  unsigned arg_index = next_arg_index_++;
  if (arg_index < args_.size())
    return args_[arg_index];
  if (!error_)
    error_ = "argument index is out of range in format";
  return DUMMY_ARG;
}

const fmt::internal::Arg &fmt::internal::FormatterBase::handle_arg_index(unsigned arg_index) {
  if (arg_index != UINT_MAX) {
    if (next_arg_index_ <= 0) {
      next_arg_index_ = -1;
      --arg_index;
    } else if (!error_) {
      error_ = "cannot switch from automatic to manual argument indexing";
    }
    if (arg_index < args_.size())
      return args_[arg_index];
    if (!error_)
      error_ = "argument index is out of range in format";
    return DUMMY_ARG;
  }
  return next_arg();
}




