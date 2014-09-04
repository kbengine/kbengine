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

#ifndef FMT_FORMAT_H_
#define FMT_FORMAT_H_

#include <stdint.h>

#include <cassert>
#include <cstddef>  // for std::ptrdiff_t
#include <cstdio>
#include <algorithm>
#include <limits>
#include <stdexcept>
#include <string>
#include <sstream>

#if _SECURE_SCL
# include <iterator>
#endif

#ifdef __GNUC__
# define FMT_GCC_VERSION (__GNUC__ * 100 + __GNUC_MINOR__)
# define FMT_GCC_EXTENSION __extension__
// Disable warning about "long long" which is sometimes reported even
// when using __extension__.
# if FMT_GCC_VERSION >= 406
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wlong-long"
# endif
#else
# define FMT_GCC_EXTENSION
#endif

#ifdef __GNUC_LIBSTD__
# define FMT_GNUC_LIBSTD_VERSION (__GNUC_LIBSTD__ * 100 + __GNUC_LIBSTD_MINOR__)
#endif

#ifdef __has_feature
# define FMT_HAS_FEATURE(x) __has_feature(x)
#else
# define FMT_HAS_FEATURE(x) 0
#endif

#ifdef __has_builtin
# define FMT_HAS_BUILTIN(x) __has_builtin(x)
#else
# define FMT_HAS_BUILTIN(x) 0
#endif

#ifndef FMT_USE_VARIADIC_TEMPLATES
// Variadic templates are available in GCC since version 4.4
// (http://gcc.gnu.org/projects/cxx0x.html) and in Visual C++
// since version 2013.
# define FMT_USE_VARIADIC_TEMPLATES \
   (FMT_HAS_FEATURE(cxx_variadic_templates) || \
       (FMT_GCC_VERSION >= 404 && __cplusplus >= 201103) || _MSC_VER >= 1800)
#endif

#ifndef FMT_USE_RVALUE_REFERENCES
// Don't use rvalue references when compiling with clang and an old libstdc++
// as the latter doesn't provide std::move.
# if defined(FMT_GNUC_LIBSTD_VERSION) && FMT_GNUC_LIBSTD_VERSION <= 402
#  define FMT_USE_RVALUE_REFERENCES 0
# else
#  define FMT_USE_RVALUE_REFERENCES \
    (FMT_HAS_FEATURE(cxx_rvalue_references) || \
        (FMT_GCC_VERSION >= 403 && __cplusplus >= 201103) || _MSC_VER >= 1600)
# endif
#endif

#if FMT_USE_RVALUE_REFERENCES
# include <utility>  // for std::move
#endif

// Define FMT_USE_NOEXCEPT to make C++ Format use noexcept (C++11 feature).
#if FMT_USE_NOEXCEPT || FMT_HAS_FEATURE(cxx_noexcept) || \
  (FMT_GCC_VERSION >= 408 && __cplusplus >= 201103)
# define FMT_NOEXCEPT(expr) noexcept(expr)
#else
# define FMT_NOEXCEPT(expr)
#endif

// A macro to disallow the copy constructor and operator= functions
// This should be used in the private: declarations for a class
#define FMT_DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&); \
  void operator=(const TypeName&)

namespace fmt {

// Fix the warning about long long on older versions of GCC
// that don't support the diagnostic pragma.
FMT_GCC_EXTENSION typedef long long LongLong;
FMT_GCC_EXTENSION typedef unsigned long long ULongLong;

#if FMT_USE_RVALUE_REFERENCES
using std::move;
#endif

template <typename Char>
class BasicWriter;

typedef BasicWriter<char> Writer;
typedef BasicWriter<wchar_t> WWriter;

template <typename Char>
class BasicFormatter;

template <typename Char, typename T>
void format(BasicFormatter<Char> &f, const Char *format_str, const T &value);

/**
  \rst
  A string reference. It can be constructed from a C string or
  ``std::string``.
  
  You can use one of the following typedefs for common character types:

  +------------+-------------------------+
  | Type       | Definition              |
  +============+=========================+
  | StringRef  | BasicStringRef<char>    |
  +------------+-------------------------+
  | WStringRef | BasicStringRef<wchar_t> |
  +------------+-------------------------+

  This class is most useful as a parameter type to allow passing
  different types of strings to a function, for example::

    template<typename... Args>
    std::string format(StringRef format, const Args & ... args);

    format("{}", 42);
    format(std::string("{}"), 42);
  \endrst
 */
template <typename Char>
class BasicStringRef {
 private:
  const Char *data_;
  mutable std::size_t size_;

 public:
  /**
    Constructs a string reference object from a C string and a size.
    If *size* is zero, which is the default, the size is computed
    automatically.
   */
  BasicStringRef(const Char *s, std::size_t size = 0) : data_(s), size_(size) {}

  /**
    Constructs a string reference from an `std::string` object.
   */
  BasicStringRef(const std::basic_string<Char> &s)
  : data_(s.c_str()), size_(s.size()) {}

  /**
    Converts a string reference to an `std::string` object.
   */
  operator std::basic_string<Char>() const {
    return std::basic_string<Char>(data_, size());
  }

  /**
    Returns the pointer to a C string.
   */
  const Char *c_str() const { return data_; }

  /**
    Returns the string size.
   */
  std::size_t size() const {
    if (size_ == 0 && data_) size_ = std::char_traits<Char>::length(data_);
    return size_;
  }

  friend bool operator==(BasicStringRef lhs, BasicStringRef rhs) {
    return lhs.data_ == rhs.data_;
  }
  friend bool operator!=(BasicStringRef lhs, BasicStringRef rhs) {
    return lhs.data_ != rhs.data_;
  }
};

typedef BasicStringRef<char> StringRef;
typedef BasicStringRef<wchar_t> WStringRef;

/**
  A formatting error such as invalid format string.
*/
class FormatError : public std::runtime_error {
public:
  explicit FormatError(const std::string &message)
  : std::runtime_error(message) {}
};

namespace internal {

// The number of characters to store in the Array object, representing the
// output buffer, itself to avoid dynamic memory allocation.
enum { INLINE_BUFFER_SIZE = 500 };

#if _SECURE_SCL
// Use checked iterator to avoid warnings on MSVC.
template <typename T>
inline stdext::checked_array_iterator<T*> make_ptr(T *ptr, std::size_t size) {
  return stdext::checked_array_iterator<T*>(ptr, size);
}
#else
template <typename T>
inline T *make_ptr(T *ptr, std::size_t) { return ptr; }
#endif

// A simple array for POD types with the first SIZE elements stored in
// the object itself. It supports a subset of std::vector's operations.
template <typename T, std::size_t SIZE>
class Array {
 private:
  std::size_t size_;
  std::size_t capacity_;
  T *ptr_;
  T data_[SIZE];

  void grow(std::size_t size);

  // Free memory allocated by the array.
  void free() {
    if (ptr_ != data_) delete [] ptr_;
  }

  // Move data from other to this array.
  void move(Array &other) {
    size_ = other.size_;
    capacity_ = other.capacity_;
    if (other.ptr_ == other.data_) {
      ptr_ = data_;
      std::copy(other.data_, other.data_ + size_, make_ptr(data_, capacity_));
    } else {
      ptr_ = other.ptr_;
      // Set pointer to the inline array so that delete is not called
      // when freeing.
      other.ptr_ = other.data_;
    }
  }

  FMT_DISALLOW_COPY_AND_ASSIGN(Array);

 public:
  explicit Array(std::size_t size = 0)
    : size_(size), capacity_(SIZE), ptr_(data_) {}
  ~Array() { free(); }

#if FMT_USE_RVALUE_REFERENCES
  Array(Array &&other) {
    move(other);
  }

  Array& operator=(Array &&other) {
    assert(this != &other);
    free();
    move(other);
    return *this;
  }
#endif

  // Returns the size of this array.
  std::size_t size() const { return size_; }

  // Returns the capacity of this array.
  std::size_t capacity() const { return capacity_; }

  // Resizes the array. If T is a POD type new elements are not initialized.
  void resize(std::size_t new_size) {
    if (new_size > capacity_)
      grow(new_size);
    size_ = new_size;
  }

  // Reserves space to store at least capacity elements.
  void reserve(std::size_t capacity) {
    if (capacity > capacity_)
      grow(capacity);
  }

  void clear() { size_ = 0; }

  void push_back(const T &value) {
    if (size_ == capacity_)
      grow(size_ + 1);
    ptr_[size_++] = value;
  }

  // Appends data to the end of the array.
  void append(const T *begin, const T *end);

  T &operator[](std::size_t index) { return ptr_[index]; }
  const T &operator[](std::size_t index) const { return ptr_[index]; }
};

template <typename T, std::size_t SIZE>
void Array<T, SIZE>::grow(std::size_t size) {
  capacity_ = (std::max)(size, capacity_ + capacity_ / 2);
  T *p = new T[capacity_];
  std::copy(ptr_, ptr_ + size_, make_ptr(p, capacity_));
  if (ptr_ != data_)
    delete [] ptr_;
  ptr_ = p;
}

template <typename T, std::size_t SIZE>
void Array<T, SIZE>::append(const T *begin, const T *end) {
  std::ptrdiff_t num_elements = end - begin;
  if (size_ + num_elements > capacity_)
    grow(size_ + num_elements);
  std::copy(begin, end, make_ptr(ptr_, capacity_) + size_);
  size_ += num_elements;
}

template <typename Char>
class BasicCharTraits {
 public:
#if _SECURE_SCL
  typedef stdext::checked_array_iterator<Char*> CharPtr;
#else
  typedef Char *CharPtr;
#endif
};

template <typename Char>
class CharTraits;

template <>
class CharTraits<char> : public BasicCharTraits<char> {
 private:
  // Conversion from wchar_t to char is not allowed.
  static char convert(wchar_t);

public:
  typedef const wchar_t *UnsupportedStrType;

  static char convert(char value) { return value; }

  // Formats a floating-point number.
  template <typename T>
  static int format_float(char *buffer, std::size_t size,
      const char *format, unsigned width, int precision, T value);
};

template <>
class CharTraits<wchar_t> : public BasicCharTraits<wchar_t> {
 public:
  typedef const char *UnsupportedStrType;

  static wchar_t convert(char value) { return value; }
  static wchar_t convert(wchar_t value) { return value; }

  template <typename T>
  static int format_float(wchar_t *buffer, std::size_t size,
      const wchar_t *format, unsigned width, int precision, T value);
};

// Selects uint32_t if FitsIn32Bits is true, uint64_t otherwise.
template <bool FitsIn32Bits>
struct TypeSelector { typedef uint32_t Type; };

template <>
struct TypeSelector<false> { typedef uint64_t Type; };

// Checks if a number is negative - used to avoid warnings.
template <bool IsSigned>
struct SignChecker {
  template <typename T>
  static bool is_negative(T) { return false; }
};

template <>
struct SignChecker<true> {
  template <typename T>
  static bool is_negative(T value) { return value < 0; }
};

// Returns true if value is negative, false otherwise.
// Same as (value < 0) but doesn't produce warnings if T is an unsigned type.
template <typename T>
inline bool is_negative(T value) {
  return SignChecker<std::numeric_limits<T>::is_signed>::is_negative(value);
}

template <typename T>
struct IntTraits {
  // Smallest of uint32_t and uint64_t that is large enough to represent
  // all values of T.
  typedef typename
    TypeSelector<std::numeric_limits<T>::digits <= 32>::Type MainType;
};

// MakeUnsigned<T>::Type gives an unsigned type corresponding to integer type T.
template <typename T>
struct MakeUnsigned { typedef T Type; };

#define FMT_SPECIALIZE_MAKE_UNSIGNED(T, U) \
  template <> \
  struct MakeUnsigned<T> { typedef U Type; }

FMT_SPECIALIZE_MAKE_UNSIGNED(char, unsigned char);
FMT_SPECIALIZE_MAKE_UNSIGNED(signed char, unsigned char);
FMT_SPECIALIZE_MAKE_UNSIGNED(short, unsigned short);
FMT_SPECIALIZE_MAKE_UNSIGNED(int, unsigned);
FMT_SPECIALIZE_MAKE_UNSIGNED(long, unsigned long);
FMT_SPECIALIZE_MAKE_UNSIGNED(LongLong, ULongLong);

void report_unknown_type(char code, const char *type);

extern const uint32_t POWERS_OF_10_32[];
extern const uint64_t POWERS_OF_10_64[];

#if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clzll)
// Returns the number of decimal digits in n. Leading zeros are not counted
// except for n == 0 in which case count_digits returns 1.
inline unsigned count_digits(uint64_t n) {
  // Based on http://graphics.stanford.edu/~seander/bithacks.html#IntegerLog10
  // and the benchmark https://github.com/localvoid/cxx-benchmark-count-digits.
  unsigned t = (64 - __builtin_clzll(n | 1)) * 1233 >> 12;
  return t - (n < POWERS_OF_10_64[t]) + 1;
}
# if FMT_GCC_VERSION >= 400 || FMT_HAS_BUILTIN(__builtin_clz)
// Optional version of count_digits for better performance on 32-bit platforms.
inline unsigned count_digits(uint32_t n) {
  uint32_t t = (32 - __builtin_clz(n | 1)) * 1233 >> 12;
  return t - (n < POWERS_OF_10_32[t]) + 1;
}
# endif
#else
// Slower version of count_digits used when __builtin_clz is not available.
inline unsigned count_digits(uint64_t n) {
  unsigned count = 1;
  for (;;) {
    // Integer division is slow so do it for a group of four digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    if (n < 10) return count;
    if (n < 100) return count + 1;
    if (n < 1000) return count + 2;
    if (n < 10000) return count + 3;
    n /= 10000u;
    count += 4;
  }
}
#endif

extern const char DIGITS[];

// Formats a decimal unsigned integer value writing into buffer.
template <typename UInt, typename Char>
inline void format_decimal(Char *buffer, UInt value, unsigned num_digits) {
  --num_digits;
  while (value >= 100) {
    // Integer division is slow so do it for a group of two digits instead
    // of for every digit. The idea comes from the talk by Alexandrescu
    // "Three Optimization Tips for C++". See speed-test for a comparison.
    unsigned index = (value % 100) * 2;
    value /= 100;
    buffer[num_digits] = DIGITS[index + 1];
    buffer[num_digits - 1] = DIGITS[index];
    num_digits -= 2;
  }
  if (value < 10) {
    *buffer = static_cast<char>('0' + value);
    return;
  }
  unsigned index = static_cast<unsigned>(value * 2);
  buffer[1] = DIGITS[index + 1];
  buffer[0] = DIGITS[index];
}

#ifdef _WIN32
// A converter from UTF-8 to UTF-16.
// It is only provided for Windows since other systems use UTF-8.
class UTF8ToUTF16 {
 private:
  Array<wchar_t, INLINE_BUFFER_SIZE> buffer_;

 public:
  explicit UTF8ToUTF16(StringRef s);
  operator WStringRef() const { return WStringRef(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const wchar_t *c_str() const { return &buffer_[0]; }
  std::wstring str() const { return std::wstring(&buffer_[0], size()); }
};

// A converter from UTF-16 to UTF-8.
// It is only provided for Windows since other systems use UTF-8.
class UTF16ToUTF8 {
 private:
  Array<char, INLINE_BUFFER_SIZE> buffer_;

 public:
  UTF16ToUTF8() {}
  explicit UTF16ToUTF8(WStringRef s);
  operator StringRef() const { return StringRef(&buffer_[0], size()); }
  size_t size() const { return buffer_.size() - 1; }
  const char *c_str() const { return &buffer_[0]; }
  std::string str() const { return std::string(&buffer_[0], size()); }

  // Performs conversion returning a system error code instead of
  // throwing exception on error.
  int convert(WStringRef s);
};
#endif

// Portable thread-safe version of strerror.
// Sets buffer to point to a string describing the error code.
// This can be either a pointer to a string stored in buffer,
// or a pointer to some static immutable string.
// Returns one of the following values:
//   0      - success
//   ERANGE - buffer is not large enough to store the error message
//   other  - failure
// Buffer should be at least of size 1.
int safe_strerror(int error_code,
    char *&buffer, std::size_t buffer_size) FMT_NOEXCEPT(true);

void format_system_error(
    fmt::Writer &out, int error_code, fmt::StringRef message);

#ifdef _WIN32
void format_windows_error(
    fmt::Writer &out, int error_code, fmt::StringRef message);
#endif

// Throws Exception(message) if format contains '}', otherwise throws
// FormatError reporting unmatched '{'. The idea is that unmatched '{'
// should override other errors.
template <typename Char>
struct FormatErrorReporter {
  int num_open_braces;
  void operator()(const Char *s, fmt::StringRef message) const;
};

// Computes max(Arg, 1) at compile time. It is used to avoid errors about
// allocating an array of 0 size.
template <unsigned Arg>
struct NonZero {
  enum { VALUE = Arg };
};

template <>
struct NonZero<0> {
  enum { VALUE = 1 };
};

// A formatting argument. It is a POD type to allow storage in internal::Array.
struct Arg {
  enum Type {
    // Integer types should go first,
    INT, UINT, LONG_LONG, ULONG_LONG, CHAR, LAST_INTEGER_TYPE = CHAR,
    // followed by floating-point types.
    DOUBLE, LONG_DOUBLE, LAST_NUMERIC_TYPE = LONG_DOUBLE,
    STRING, WSTRING, POINTER, CUSTOM
  };
  Type type;

  template <typename Char>
  struct StringValue {
    const Char *value;
    std::size_t size;
  };

  typedef void (*FormatFunc)(
      void *formatter, const void *arg, const void *format_str);

  struct CustomValue {
    const void *value;
    FormatFunc format;
  };

  union {
    int int_value;
    unsigned uint_value;
    LongLong long_long_value;
    ULongLong ulong_long_value;
    double double_value;
    long double long_double_value;
    const void *pointer_value;
    StringValue<char> string;
    StringValue<wchar_t> wstring;
    CustomValue custom;
  };
};

// Makes an Arg object from any type.
template <typename Char>
class MakeArg : public Arg {
 private:
  // The following two methods are private to disallow formatting of
  // arbitrary pointers. If you want to output a pointer cast it to
  // "void *" or "const void *". In particular, this forbids formatting
  // of "[const] volatile char *" which is printed as bool by iostreams.
  // Do not implement!
  template <typename T>
  MakeArg(const T *value);
  template <typename T>
  MakeArg(T *value);

  void set_string(StringRef str) {
    type = STRING;
    string.value = str.c_str();
    string.size = str.size();
  }

  void set_string(WStringRef str) {
    type = WSTRING;
    CharTraits<Char>::convert(wchar_t());
    wstring.value = str.c_str();
    wstring.size = str.size();
  }

  // Formats an argument of a custom type, such as a user-defined class.
  template <typename T>
  static void format_custom_arg(
      void *formatter, const void *arg, const void *format_str) {
    format(*static_cast<BasicFormatter<Char>*>(formatter),
        static_cast<const Char*>(format_str), *static_cast<const T*>(arg));
  }

public:
  MakeArg() {}
  MakeArg(bool value) { type = INT; int_value = value; }
  MakeArg(short value) { type = INT; int_value = value; }
  MakeArg(unsigned short value) { type = UINT; uint_value = value; }
  MakeArg(int value) { type = INT; int_value = value; }
  MakeArg(unsigned value) { type = UINT; uint_value = value; }
  MakeArg(long value) {
    // To minimize the number of types we need to deal with, long is
    // translated either to int or to long long depending on its size.
    if (sizeof(long) == sizeof(int)) {
      type = INT;
      int_value = static_cast<int>(value);
    } else {
      type = LONG_LONG;
      long_long_value = value;
    }
  }
  MakeArg(unsigned long value) {
    if (sizeof(unsigned long) == sizeof(unsigned)) {
      type = UINT;
      uint_value = static_cast<unsigned>(value);
    } else {
      type = ULONG_LONG;
      ulong_long_value = value;
    }
  }
  MakeArg(LongLong value) { type = LONG_LONG; long_long_value = value; }
  MakeArg(ULongLong value) { type = ULONG_LONG; ulong_long_value = value; }
  MakeArg(float value) { type = DOUBLE; double_value = value; }
  MakeArg(double value) { type = DOUBLE; double_value = value; }
  MakeArg(long double value) { type = LONG_DOUBLE; long_double_value = value; }
  MakeArg(signed char value) { type = CHAR; int_value = value; }
  MakeArg(unsigned char value) { type = CHAR; int_value = value; }
  MakeArg(char value) { type = CHAR; int_value = value; }
  MakeArg(wchar_t value) {
    type = CHAR;
    int_value = internal::CharTraits<Char>::convert(value);
  }

  MakeArg(char *value) { set_string(value); }
  MakeArg(const char *value) { set_string(value); }
  MakeArg(const std::string &value) { set_string(value); }
  MakeArg(StringRef value) { set_string(value); }

  MakeArg(wchar_t *value) { set_string(value); }
  MakeArg(const wchar_t *value) { set_string(value); }
  MakeArg(const std::wstring &value) { set_string(value); }
  MakeArg(WStringRef value) { set_string(value); }

  MakeArg(void *value) { type = POINTER; pointer_value = value; }
  MakeArg(const void *value) { type = POINTER; pointer_value = value; }

  template <typename T>
  MakeArg(const T &value) {
    type = CUSTOM;
    custom.value = &value;
    custom.format = &format_custom_arg<T>;
  }
};

#define FMT_DISPATCH(call) static_cast<Impl*>(this)->call

// An argument visitor.
// To use ArgVisitor define a subclass that implements some or all of the
// visit methods with the same signatures as the methods in ArgVisitor,
// for example, visit_int(int).
// Specify the subclass name as the Impl template parameter. Then calling
// ArgVisitor::visit for some argument will dispatch to a visit method
// specific to the argument type. For example, if the argument type is
// double then visit_double(double) method of a subclass will be called.
// If the subclass doesn't contain a method with this signature, then
// a corresponding method of ArgVisitor will be called.
//
// Example:
//  class MyArgVisitor : public ArgVisitor<MyArgVisitor, void> {
//   public:
//    void visit_int(int value) { print("{}", value); }
//    void visit_double(double value) { print("{}", value ); }
//  };
//
// ArgVisitor uses the curiously recurring template pattern:
// http://en.wikipedia.org/wiki/Curiously_recurring_template_pattern
template <typename Impl, typename Result>
class ArgVisitor {
 public:
  Result visit_unhandled_arg() { return Result(); }

  Result visit_int(int value) {
    return FMT_DISPATCH(visit_any_int(value));
  }
  Result visit_long_long(LongLong value) {
    return FMT_DISPATCH(visit_any_int(value));
  }
  Result visit_uint(unsigned value) {
    return FMT_DISPATCH(visit_any_int(value));
  }
  Result visit_ulong_long(ULongLong value) {
    return FMT_DISPATCH(visit_any_int(value));
  }
  Result visit_char(int value) {
    return FMT_DISPATCH(visit_any_int(value));
  }
  template <typename T>
  Result visit_any_int(T) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }

  Result visit_double(double value) {
    return FMT_DISPATCH(visit_any_double(value));
  }
  Result visit_long_double(long double value) {
    return FMT_DISPATCH(visit_any_double(value));
  }
  template <typename T>
  Result visit_any_double(T) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }

  Result visit_string(Arg::StringValue<char>) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }
  Result visit_wstring(Arg::StringValue<wchar_t>) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }
  Result visit_pointer(const void *) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }
  Result visit_custom(Arg::CustomValue) {
    return FMT_DISPATCH(visit_unhandled_arg());
  }

  Result visit(const Arg &arg) {
    switch (arg.type) {
    default:
      assert(false);
      // Fall through.
    case Arg::INT:
      return FMT_DISPATCH(visit_int(arg.int_value));
    case Arg::UINT:
      return FMT_DISPATCH(visit_uint(arg.uint_value));
    case Arg::LONG_LONG:
      return FMT_DISPATCH(visit_long_long(arg.long_long_value));
    case Arg::ULONG_LONG:
      return FMT_DISPATCH(visit_ulong_long(arg.ulong_long_value));
    case Arg::DOUBLE:
      return FMT_DISPATCH(visit_double(arg.double_value));
    case Arg::LONG_DOUBLE:
      return FMT_DISPATCH(visit_long_double(arg.long_double_value));
    case Arg::CHAR:
      return FMT_DISPATCH(visit_char(arg.int_value));
    case Arg::STRING:
      return FMT_DISPATCH(visit_string(arg.string));
    case Arg::WSTRING:
      return FMT_DISPATCH(visit_wstring(arg.wstring));
    case Arg::POINTER:
      return FMT_DISPATCH(visit_pointer(arg.pointer_value));
    case Arg::CUSTOM:
      return FMT_DISPATCH(visit_custom(arg.custom));
    }
  }
};

class RuntimeError : public std::runtime_error {
 protected:
  RuntimeError() : std::runtime_error("") {}
};

template <typename Char>
class ArgFormatter;
}  // namespace internal

/**
  An argument list.
 */
class ArgList {
 private:
  const internal::Arg *args_;
  std::size_t size_;

 public:
  ArgList() : size_(0) {}
  ArgList(const internal::Arg *args, std::size_t size)
  : args_(args), size_(size) {}

  /**
    Returns the list size (the number of arguments).
   */
  std::size_t size() const { return size_; }

  /**
    Returns the argument at specified index.
   */
  const internal::Arg &operator[](std::size_t index) const {
    return args_[index];
  }
};

struct FormatSpec;

namespace internal {

class FormatterBase {
protected:
  ArgList args_;
  int next_arg_index_;
  const char *error_;

  FormatterBase() : error_(0) {}

  const Arg &next_arg();

  const Arg &handle_arg_index(unsigned arg_index);

  template <typename Char>
  void write(BasicWriter<Char> &w, const Char *start, const Char *end) {
    if (start != end)
      w << BasicStringRef<Char>(start, end - start);
  }

  // TODO
};

// A printf formatter.
template <typename Char>
class PrintfFormatter : private FormatterBase {
 private:
  void parse_flags(FormatSpec &spec, const Char *&s);

  // Parses argument index, flags and width and returns the parsed
  // argument index.
  unsigned parse_header(const Char *&s, FormatSpec &spec);

 public:
  void format(BasicWriter<Char> &writer,
    BasicStringRef<Char> format, const ArgList &args);
};
}  // namespace internal

// A formatter.
template <typename Char>
class BasicFormatter : private internal::FormatterBase {
private:
  BasicWriter<Char> &writer_;
  const Char *start_;
  internal::FormatErrorReporter<Char> report_error_;

  // Parses argument index and returns an argument with this index.
  const internal::Arg &parse_arg_index(const Char *&s);

  void check_sign(const Char *&s, const internal::Arg &arg);

public:
  explicit BasicFormatter(BasicWriter<Char> &w) : writer_(w) {}

  BasicWriter<Char> &writer() { return writer_; }

  void format(BasicStringRef<Char> format_str, const ArgList &args);

  const Char *format(const Char *format_str, const internal::Arg &arg);
};

enum Alignment {
  ALIGN_DEFAULT, ALIGN_LEFT, ALIGN_RIGHT, ALIGN_CENTER, ALIGN_NUMERIC
};

// Flags.
enum {
  SIGN_FLAG = 1, PLUS_FLAG = 2, MINUS_FLAG = 4, HASH_FLAG = 8,
  CHAR_FLAG = 0x10  // Argument has char type - used in error reporting.
};

// An empty format specifier.
struct EmptySpec {};

// A type specifier.
template <char TYPE>
struct TypeSpec : EmptySpec {
  Alignment align() const { return ALIGN_DEFAULT; }
  unsigned width() const { return 0; }
  int precision() const { return -1; }
  bool flag(unsigned) const { return false; }
  char type() const { return TYPE; }
  char fill() const { return ' '; }
};

// A width specifier.
struct WidthSpec {
  unsigned width_;
  // Fill is always wchar_t and cast to char if necessary to avoid having
  // two specialization of WidthSpec and its subclasses.
  wchar_t fill_;

  WidthSpec(unsigned width, wchar_t fill) : width_(width), fill_(fill) {}

  unsigned width() const { return width_; }
  wchar_t fill() const { return fill_; }
};

// An alignment specifier.
struct AlignSpec : WidthSpec {
  Alignment align_;

  AlignSpec(unsigned width, wchar_t fill, Alignment align = ALIGN_DEFAULT)
  : WidthSpec(width, fill), align_(align) {}

  Alignment align() const { return align_; }

  int precision() const { return -1; }
};

// An alignment and type specifier.
template <char TYPE>
struct AlignTypeSpec : AlignSpec {
  AlignTypeSpec(unsigned width, wchar_t fill) : AlignSpec(width, fill) {}

  bool flag(unsigned) const { return false; }
  char type() const { return TYPE; }
};

// A full format specifier.
struct FormatSpec : AlignSpec {
  unsigned flags_;
  int precision_;
  char type_;

  FormatSpec(
    unsigned width = 0, char type = 0, wchar_t fill = ' ')
  : AlignSpec(width, fill), flags_(0), precision_(-1), type_(type) {}

  bool flag(unsigned f) const { return (flags_ & f) != 0; }
  int precision() const { return precision_; }
  char type() const { return type_; }
};

// An integer format specifier.
template <typename T, typename SpecT = TypeSpec<0>, typename Char = char>
class IntFormatSpec : public SpecT {
 private:
  T value_;

 public:
  IntFormatSpec(T value, const SpecT &spec = SpecT())
  : SpecT(spec), value_(value) {}

  T value() const { return value_; }
};

// A string format specifier.
template <typename T>
class StrFormatSpec : public AlignSpec {
 private:
  const T *str_;

 public:
  StrFormatSpec(const T *str, unsigned width, wchar_t fill)
  : AlignSpec(width, fill), str_(str) {}

  const T *str() const { return str_; }
};

/**
  Returns an integer format specifier to format the value in base 2.
 */
IntFormatSpec<int, TypeSpec<'b'> > bin(int value);

/**
  Returns an integer format specifier to format the value in base 8.
 */
IntFormatSpec<int, TypeSpec<'o'> > oct(int value);

/**
  Returns an integer format specifier to format the value in base 16 using
  lower-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'x'> > hex(int value);

/**
  Returns an integer formatter format specifier to format in base 16 using
  upper-case letters for the digits above 9.
 */
IntFormatSpec<int, TypeSpec<'X'> > hexu(int value);

/**
  \rst
  Returns an integer format specifier to pad the formatted argument with the
  fill character to the specified width using the default (right) numeric
  alignment.

  **Example**::

    Writer out;
    out << pad(hex(0xcafe), 8, '0');
    // out.str() == "0000cafe"

  \endrst
 */
template <char TYPE_CODE, typename Char>
IntFormatSpec<int, AlignTypeSpec<TYPE_CODE>, Char> pad(
    int value, unsigned width, Char fill = ' ');

#define FMT_DEFINE_INT_FORMATTERS(TYPE) \
inline IntFormatSpec<TYPE, TypeSpec<'b'> > bin(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'b'> >(value, TypeSpec<'b'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'o'> > oct(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'o'> >(value, TypeSpec<'o'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'x'> > hex(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'x'> >(value, TypeSpec<'x'>()); \
} \
 \
inline IntFormatSpec<TYPE, TypeSpec<'X'> > hexu(TYPE value) { \
  return IntFormatSpec<TYPE, TypeSpec<'X'> >(value, TypeSpec<'X'>()); \
} \
 \
template <char TYPE_CODE> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> > pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE> > f, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE> >( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, ' ')); \
} \
 \
/* For compatibility with older compilers we provide two overloads for pad, */ \
/* one that takes a fill character and one that doesn't. In the future this */ \
/* can be replaced with one overload making the template argument Char      */ \
/* default to char (C++11). */ \
template <char TYPE_CODE, typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char> pad( \
    IntFormatSpec<TYPE, TypeSpec<TYPE_CODE>, Char> f, \
    unsigned width, Char fill) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<TYPE_CODE>, Char>( \
      f.value(), AlignTypeSpec<TYPE_CODE>(width, fill)); \
} \
 \
inline IntFormatSpec<TYPE, AlignTypeSpec<0> > pad( \
    TYPE value, unsigned width) { \
  return IntFormatSpec<TYPE, AlignTypeSpec<0> >( \
      value, AlignTypeSpec<0>(width, ' ')); \
} \
 \
template <typename Char> \
inline IntFormatSpec<TYPE, AlignTypeSpec<0>, Char> pad( \
   TYPE value, unsigned width, Char fill) { \
 return IntFormatSpec<TYPE, AlignTypeSpec<0>, Char>( \
     value, AlignTypeSpec<0>(width, fill)); \
}

FMT_DEFINE_INT_FORMATTERS(int)
FMT_DEFINE_INT_FORMATTERS(long)
FMT_DEFINE_INT_FORMATTERS(unsigned)
FMT_DEFINE_INT_FORMATTERS(unsigned long)
FMT_DEFINE_INT_FORMATTERS(LongLong)
FMT_DEFINE_INT_FORMATTERS(ULongLong)

/**
  \rst
  Returns a string formatter that pads the formatted argument with the fill
  character to the specified width using the default (left) string alignment.

  **Example**::

    std::string s = str(Writer() << pad("abc", 8));
    // s == "abc     "

  \endrst
 */
template <typename Char>
inline StrFormatSpec<Char> pad(
    const Char *str, unsigned width, Char fill = ' ') {
  return StrFormatSpec<Char>(str, width, fill);
}

inline StrFormatSpec<wchar_t> pad(
    const wchar_t *str, unsigned width, char fill = ' ') {
  return StrFormatSpec<wchar_t>(str, width, fill);
}

// Generates a comma-separated list with results of applying f to numbers 0..n-1.
# define FMT_GEN(n, f) FMT_GEN##n(f)
# define FMT_GEN1(f)  f(0)
# define FMT_GEN2(f)  FMT_GEN1(f), f(1)
# define FMT_GEN3(f)  FMT_GEN2(f), f(2)
# define FMT_GEN4(f)  FMT_GEN3(f), f(3)
# define FMT_GEN5(f)  FMT_GEN4(f), f(4)
# define FMT_GEN6(f)  FMT_GEN5(f), f(5)
# define FMT_GEN7(f)  FMT_GEN6(f), f(6)
# define FMT_GEN8(f)  FMT_GEN7(f), f(7)
# define FMT_GEN9(f)  FMT_GEN8(f), f(8)
# define FMT_GEN10(f) FMT_GEN9(f), f(9)

# define FMT_MAKE_TEMPLATE_ARG(n) typename T##n
# define FMT_MAKE_ARG(n) const T##n &v##n
# define FMT_MAKE_REF_char(n) fmt::internal::MakeArg<char>(v##n)
# define FMT_MAKE_REF_wchar_t(n) fmt::internal::MakeArg<wchar_t>(v##n)

#if FMT_USE_VARIADIC_TEMPLATES
// Defines a variadic function returning void.
# define FMT_VARIADIC_VOID(func, arg_type) \
  template<typename... Args> \
  void func(arg_type arg1, const Args & ... args) { \
    using fmt::internal::Arg; \
    const Arg arg_array[fmt::internal::NonZero<sizeof...(Args)>::VALUE] = { \
      fmt::internal::MakeArg<Char>(args)... \
    }; \
    func(arg1, ArgList(arg_array, sizeof...(Args))); \
  }

// Defines a variadic constructor.
# define FMT_VARIADIC_CTOR(ctor, func, arg0_type, arg1_type) \
  template<typename... Args> \
  ctor(arg0_type arg0, arg1_type arg1, const Args & ... args) { \
    using fmt::internal::Arg; \
    const Arg arg_array[fmt::internal::NonZero<sizeof...(Args)>::VALUE] = { \
      fmt::internal::MakeArg<Char>(args)... \
    }; \
    func(arg0, arg1, ArgList(arg_array, sizeof...(Args))); \
  }

#else

# define FMT_MAKE_REF(n) fmt::internal::MakeArg<Char>(v##n)
// Defines a wrapper for a function taking one argument of type arg_type
// and n additional arguments of arbitrary types.
# define FMT_WRAP1(func, arg_type, n) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  inline void func(arg_type arg1, FMT_GEN(n, FMT_MAKE_ARG)) { \
    const fmt::internal::Arg args[] = {FMT_GEN(n, FMT_MAKE_REF)}; \
    func(arg1, fmt::ArgList(args, sizeof(args) / sizeof(*args))); \
  }

// Emulates a variadic function returning void on a pre-C++11 compiler.
# define FMT_VARIADIC_VOID(func, arg_type) \
  FMT_WRAP1(func, arg_type, 1) FMT_WRAP1(func, arg_type, 2) \
  FMT_WRAP1(func, arg_type, 3) FMT_WRAP1(func, arg_type, 4) \
  FMT_WRAP1(func, arg_type, 5) FMT_WRAP1(func, arg_type, 6) \
  FMT_WRAP1(func, arg_type, 7) FMT_WRAP1(func, arg_type, 8) \
  FMT_WRAP1(func, arg_type, 9) FMT_WRAP1(func, arg_type, 10)

# define FMT_CTOR(ctor, func, arg0_type, arg1_type, n) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  ctor(arg0_type arg0, arg1_type arg1, FMT_GEN(n, FMT_MAKE_ARG)) { \
    const fmt::internal::Arg args[] = {FMT_GEN(n, FMT_MAKE_REF)}; \
    func(arg0, arg1, fmt::ArgList(args, sizeof(args) / sizeof(*args))); \
  }

// Emulates a variadic constructor on a pre-C++11 compiler.
# define FMT_VARIADIC_CTOR(ctor, func, arg0_type, arg1_type) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 1) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 2) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 3) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 4) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 5) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 6) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 7) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 8) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 9) \
  FMT_CTOR(ctor, func, arg0_type, arg1_type, 10)
#endif

// Generates a comma-separated list with results of applying f to pairs
// (argument, index).
#define FMT_FOR_EACH1(f, x0) f(x0, 0)
#define FMT_FOR_EACH2(f, x0, x1) \
  FMT_FOR_EACH1(f, x0), f(x1, 1)
#define FMT_FOR_EACH3(f, x0, x1, x2) \
  FMT_FOR_EACH2(f, x0 ,x1), f(x2, 2)
#define FMT_FOR_EACH4(f, x0, x1, x2, x3) \
  FMT_FOR_EACH3(f, x0, x1, x2), f(x3, 3)
#define FMT_FOR_EACH5(f, x0, x1, x2, x3, x4) \
  FMT_FOR_EACH4(f, x0, x1, x2, x3), f(x4, 4)
#define FMT_FOR_EACH6(f, x0, x1, x2, x3, x4, x5) \
  FMT_FOR_EACH5(f, x0, x1, x2, x3, x4), f(x5, 5)
#define FMT_FOR_EACH7(f, x0, x1, x2, x3, x4, x5, x6) \
  FMT_FOR_EACH6(f, x0, x1, x2, x3, x4, x5), f(x6, 6)
#define FMT_FOR_EACH8(f, x0, x1, x2, x3, x4, x5, x6, x7) \
  FMT_FOR_EACH7(f, x0, x1, x2, x3, x4, x5, x6), f(x7, 7)
#define FMT_FOR_EACH9(f, x0, x1, x2, x3, x4, x5, x6, x7, x8) \
  FMT_FOR_EACH8(f, x0, x1, x2, x3, x4, x5, x6, x7), f(x8, 8)
#define FMT_FOR_EACH10(f, x0, x1, x2, x3, x4, x5, x6, x7, x8, x9) \
  FMT_FOR_EACH9(f, x0, x1, x2, x3, x4, x5, x6, x7, x8), f(x9, 9)

/**
An error returned by an operating system or a language runtime,
for example a file opening error.
*/
class SystemError : public internal::RuntimeError {
 private:
  void init(int error_code, StringRef format_str, const ArgList &args);

 protected:
  int error_code_;

  typedef char Char;  // For FMT_VARIADIC_CTOR.

  SystemError() {}

 public:
  /**
   \rst
   Constructs a :cpp:class:`fmt::SystemError` object with the description
   of the form "*<message>*: *<system-message>*", where *<message>* is the
   formatted message and *<system-message>* is the system message corresponding
   to the error code.
   *error_code* is a system error code as given by ``errno``.
   \endrst
  */
  SystemError(int error_code, StringRef message) {
    init(error_code, message, ArgList());
  }
  FMT_VARIADIC_CTOR(SystemError, init, int, StringRef)

  int error_code() const { return error_code_; }
};

/**
  \rst
  This template provides operations for formatting and writing data into
  a character stream. The output is stored in a memory buffer that grows
  dynamically.

  You can use one of the following typedefs for common character types:

  +---------+----------------------+
  | Type    | Definition           |
  +=========+======================+
  | Writer  | BasicWriter<char>    |
  +---------+----------------------+
  | WWriter | BasicWriter<wchar_t> |
  +---------+----------------------+

  **Example**::

     Writer out;
     out << "The answer is " << 42 << "\n";
     out.write("({:+f}, {:+f})", -3.14, 3.14);

  This will write the following output to the ``out`` object:

  .. code-block:: none

     The answer is 42
     (-3.140000, +3.140000)

  The output can be converted to an ``std::string`` with ``out.str()`` or
  accessed as a C string with ``out.c_str()``.
  \endrst
 */
template <typename Char>
class BasicWriter {
 private:
  // Output buffer.
  mutable internal::Array<Char, internal::INLINE_BUFFER_SIZE> buffer_;

  typedef typename internal::CharTraits<Char>::CharPtr CharPtr;

#if _SECURE_SCL
  // Returns pointer value.
  static Char *get(CharPtr p) { return p.base(); }
#else
  static Char *get(Char *p) { return p; }
#endif

  static CharPtr fill_padding(CharPtr buffer,
      unsigned total_size, std::size_t content_size, wchar_t fill);

  // Grows the buffer by n characters and returns a pointer to the newly
  // allocated area.
  CharPtr grow_buffer(std::size_t n) {
    std::size_t size = buffer_.size();
    buffer_.resize(size + n);
    return internal::make_ptr(&buffer_[size], n);
  }

  // Prepare a buffer for integer formatting.
  CharPtr prepare_int_buffer(unsigned num_digits,
      const EmptySpec &, const char *prefix, unsigned prefix_size) {
    unsigned size = prefix_size + num_digits;
    CharPtr p = grow_buffer(size);
    std::copy(prefix, prefix + prefix_size, p);
    return p + size - 1;
  }

  template <typename Spec>
  CharPtr prepare_int_buffer(unsigned num_digits,
    const Spec &spec, const char *prefix, unsigned prefix_size);

  // Formats an integer.
  template <typename T, typename Spec>
  void write_int(T value, const Spec &spec);

  // Formats a floating-point number (double or long double).
  template <typename T>
  void write_double(T value, const FormatSpec &spec);

  // Writes a formatted string.
  template <typename StrChar>
  CharPtr write_str(
      const StrChar *s, std::size_t size, const AlignSpec &spec);

  template <typename StrChar>
  void write_str(
      const internal::Arg::StringValue<StrChar> &str, const FormatSpec &spec);

  // This method is private to disallow writing a wide string to a
  // char stream and vice versa. If you want to print a wide string
  // as a pointer as std::ostream does, cast it to const void*.
  // Do not implement!
  void operator<<(typename internal::CharTraits<Char>::UnsupportedStrType);

  friend class internal::ArgFormatter<Char>;
  friend class internal::PrintfFormatter<Char>;

 public:
  /**
    Constructs a ``BasicWriter`` object.
   */
  BasicWriter() {}

#if FMT_USE_RVALUE_REFERENCES
  /**
    Constructs a ``BasicWriter`` object moving the content of the other
    object to it.
   */
  BasicWriter(BasicWriter &&other) : buffer_(std::move(other.buffer_)) {}

  /**
    Moves the content of the other ``BasicWriter`` object to this one.
   */
  BasicWriter& operator=(BasicWriter &&other) {
    assert(this != &other);
    buffer_ = std::move(other.buffer_);
    return *this;
  }
#endif

  /**
    Returns the total number of characters written.
   */
  std::size_t size() const { return buffer_.size(); }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const Char *data() const { return &buffer_[0]; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const Char *c_str() const {
    std::size_t size = buffer_.size();
    buffer_.reserve(size + 1);
    buffer_[size] = '\0';
    return &buffer_[0];
  }

  /**
    Returns the content of the output buffer as an `std::string`.
   */
  std::basic_string<Char> str() const {
    return std::basic_string<Char>(&buffer_[0], buffer_.size());
  }

  /**
    \rst
    Writes formatted data.
    
    *args* is an argument list representing arbitrary arguments.

    **Example**::

       Writer out;
       out.write("Current point:\n");
       out.write("({:+f}, {:+f})", -3.14, 3.14);

    This will write the following output to the ``out`` object:

    .. code-block:: none

       Current point:
       (-3.140000, +3.140000)

    The output can be accessed using :meth:`data`, :meth:`c_str` or :meth:`str`
    methods.

    See also `Format String Syntax`_.
    \endrst
   */
  void write(BasicStringRef<Char> format, const ArgList &args) {
    BasicFormatter<Char>(*this).format(format, args);
  }
  FMT_VARIADIC_VOID(write, fmt::BasicStringRef<Char>)

  BasicWriter &operator<<(int value) {
    return *this << IntFormatSpec<int>(value);
  }
  BasicWriter &operator<<(unsigned value) {
    return *this << IntFormatSpec<unsigned>(value);
  }
  BasicWriter &operator<<(long value) {
    return *this << IntFormatSpec<long>(value);
  }
  BasicWriter &operator<<(unsigned long value) {
    return *this << IntFormatSpec<unsigned long>(value);
  }
  BasicWriter &operator<<(LongLong value) {
    return *this << IntFormatSpec<LongLong>(value);
  }

  /**
    Formats *value* and writes it to the stream.
   */
  BasicWriter &operator<<(ULongLong value) {
    return *this << IntFormatSpec<ULongLong>(value);
  }

  BasicWriter &operator<<(double value) {
    write_double(value, FormatSpec());
    return *this;
  }

  /**
    Formats *value* using the general format for floating-point numbers
    (``'g'``) and writes it to the stream.
   */
  BasicWriter &operator<<(long double value) {
    write_double(value, FormatSpec());
    return *this;
  }

  /**
    Writes a character to the stream.
   */
  BasicWriter &operator<<(char value) {
    buffer_.push_back(value);
    return *this;
  }

  BasicWriter &operator<<(wchar_t value) {
    buffer_.push_back(internal::CharTraits<Char>::convert(value));
    return *this;
  }

  /**
    Writes *value* to the stream.
   */
  BasicWriter &operator<<(fmt::BasicStringRef<Char> value) {
    const Char *str = value.c_str();
    buffer_.append(str, str + value.size());
    return *this;
  }

  template <typename T, typename Spec, typename FillChar>
  BasicWriter &operator<<(const IntFormatSpec<T, Spec, FillChar> &spec) {
    internal::CharTraits<Char>::convert(FillChar());
    write_int(spec.value(), spec);
    return *this;
  }

  template <typename StrChar>
  BasicWriter &operator<<(const StrFormatSpec<StrChar> &spec) {
    const StrChar *s = spec.str();
    // TODO: error if fill is not convertible to Char
    write_str(s, std::char_traits<Char>::length(s), spec);
    return *this;
  }

  void clear() { buffer_.clear(); }
};

template <typename Char>
template <typename StrChar>
typename BasicWriter<Char>::CharPtr BasicWriter<Char>::write_str(
    const StrChar *s, std::size_t size, const AlignSpec &spec) {
  CharPtr out = CharPtr();
  if (spec.width() > size) {
    out = grow_buffer(spec.width());
    Char fill = static_cast<Char>(spec.fill());
    if (spec.align() == ALIGN_RIGHT) {
      std::fill_n(out, spec.width() - size, fill);
      out += spec.width() - size;
    } else if (spec.align() == ALIGN_CENTER) {
      out = fill_padding(out, spec.width(), size, fill);
    } else {
      std::fill_n(out + size, spec.width() - size, fill);
    }
  } else {
    out = grow_buffer(size);
  }
  std::copy(s, s + size, out);
  return out;
}

template <typename Char>
template <typename Spec>
typename fmt::BasicWriter<Char>::CharPtr
  fmt::BasicWriter<Char>::prepare_int_buffer(
    unsigned num_digits, const Spec &spec,
    const char *prefix, unsigned prefix_size) {
  unsigned width = spec.width();
  Alignment align = spec.align();
  Char fill = static_cast<Char>(spec.fill());
  if (spec.precision() > static_cast<int>(num_digits)) {
    // Octal prefix '0' is counted as a digit, so ignore it if precision
    // is specified.
    if (prefix_size > 0 && prefix[prefix_size - 1] == '0')
      --prefix_size;
    unsigned number_size = prefix_size + spec.precision();
    AlignSpec subspec(number_size, '0', ALIGN_NUMERIC);
    if (number_size >= width)
      return prepare_int_buffer(num_digits, subspec, prefix, prefix_size);
    buffer_.reserve(width);
    unsigned fill_size = width - number_size;
    if (align != ALIGN_LEFT) {
      CharPtr p = grow_buffer(fill_size);
      std::fill(p, p + fill_size, fill);
    }
    CharPtr result = prepare_int_buffer(
        num_digits, subspec, prefix, prefix_size);
    if (align == ALIGN_LEFT) {
      CharPtr p = grow_buffer(fill_size);
      std::fill(p, p + fill_size, fill);
    }
    return result;
  }
  unsigned size = prefix_size + num_digits;
  if (width <= size) {
    CharPtr p = grow_buffer(size);
    std::copy(prefix, prefix + prefix_size, p);
    return p + size - 1;
  }
  CharPtr p = grow_buffer(width);
  CharPtr end = p + width;
  if (align == ALIGN_LEFT) {
    std::copy(prefix, prefix + prefix_size, p);
    p += size;
    std::fill(p, end, fill);
  } else if (align == ALIGN_CENTER) {
    p = fill_padding(p, width, size, fill);
    std::copy(prefix, prefix + prefix_size, p);
    p += size;
  } else {
    if (align == ALIGN_NUMERIC) {
      if (prefix_size != 0) {
        p = std::copy(prefix, prefix + prefix_size, p);
        size -= prefix_size;
      }
    } else {
      std::copy(prefix, prefix + prefix_size, end - size);
    }
    std::fill(p, end - size, fill);
    p = end;
  }
  return p - 1;
}

template <typename Char>
template <typename T, typename Spec>
void BasicWriter<Char>::write_int(T value, const Spec &spec) {
  unsigned prefix_size = 0;
  typedef typename internal::IntTraits<T>::MainType UnsignedType;
  UnsignedType abs_value = value;
  char prefix[4] = "";
  if (internal::is_negative(value)) {
    prefix[0] = '-';
    ++prefix_size;
    abs_value = 0 - abs_value;
  } else if (spec.flag(SIGN_FLAG)) {
    prefix[0] = spec.flag(PLUS_FLAG) ? '+' : ' ';
    ++prefix_size;
  }
  switch (spec.type()) {
  case 0: case 'd': {
    unsigned num_digits = internal::count_digits(abs_value);
    CharPtr p = prepare_int_buffer(
      num_digits, spec, prefix, prefix_size) + 1 - num_digits;
    internal::format_decimal(get(p), abs_value, num_digits);
    break;
  }
  case 'x': case 'X': {
    UnsignedType n = abs_value;
    if (spec.flag(HASH_FLAG)) {
      prefix[prefix_size++] = '0';
      prefix[prefix_size++] = spec.type();
    }
    unsigned num_digits = 0;
    do {
      ++num_digits;
    } while ((n >>= 4) != 0);
    Char *p = get(prepare_int_buffer(
      num_digits, spec, prefix, prefix_size));
    n = abs_value;
    const char *digits = spec.type() == 'x' ?
        "0123456789abcdef" : "0123456789ABCDEF";
    do {
      *p-- = digits[n & 0xf];
    } while ((n >>= 4) != 0);
    break;
  }
  case 'b': case 'B': {
    UnsignedType n = abs_value;
    if (spec.flag(HASH_FLAG)) {
      prefix[prefix_size++] = '0';
      prefix[prefix_size++] = spec.type();
    }
    unsigned num_digits = 0;
    do {
      ++num_digits;
    } while ((n >>= 1) != 0);
    Char *p = get(prepare_int_buffer(num_digits, spec, prefix, prefix_size));
    n = abs_value;
    do {
      *p-- = '0' + (n & 1);
    } while ((n >>= 1) != 0);
    break;
  }
  case 'o': {
    UnsignedType n = abs_value;
    if (spec.flag(HASH_FLAG))
      prefix[prefix_size++] = '0';
    unsigned num_digits = 0;
    do {
      ++num_digits;
    } while ((n >>= 3) != 0);
    Char *p = get(prepare_int_buffer(num_digits, spec, prefix, prefix_size));
    n = abs_value;
    do {
      *p-- = '0' + (n & 7);
    } while ((n >>= 3) != 0);
    break;
  }
  default:
    internal::report_unknown_type(
      spec.type(), spec.flag(CHAR_FLAG) ? "char" : "integer");
    break;
  }
}

// Formats a value.
template <typename Char, typename T>
void format(BasicFormatter<Char> &f, const Char *format_str, const T &value) {
  std::basic_ostringstream<Char> os;
  os << value;
  f.format(format_str, internal::MakeArg<Char>(os.str()));
}

// Reports a system error without throwing an exception.
// Can be used to report errors from destructors.
void report_system_error(int error_code, StringRef message) FMT_NOEXCEPT(true);

#ifdef _WIN32

/**
 A Windows error.
*/
class WindowsError : public SystemError {
 private:
  void init(int error_code, StringRef format_str, const ArgList &args);

 public:
  /**
   \rst
   Constructs a :cpp:class:`fmt::WindowsError` object with the description
   of the form "*<message>*: *<system-message>*", where *<message>* is the
   formatted message and *<system-message>* is the system message corresponding
   to the error code.
   *error_code* is a Windows error code as given by ``GetLastError``.
   \endrst
  */
  WindowsError(int error_code, StringRef message) {
    init(error_code, message, ArgList());
  }
  FMT_VARIADIC_CTOR(WindowsError, init, int, StringRef)
};

// Reports a Windows error without throwing an exception.
// Can be used to report errors from destructors.
void report_windows_error(int error_code, StringRef message) FMT_NOEXCEPT(true);

#endif

enum Color { BLACK, RED, GREEN, YELLOW, BLUE, MAGENTA, CYAN, WHITE };

/**
  Formats a string and prints it to stdout using ANSI escape sequences
  to specify color (experimental).
  Example:
    PrintColored(fmt::RED, "Elapsed time: {0:.2f} seconds") << 1.23;
 */
void print_colored(Color c, StringRef format, const ArgList &args);

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = format("The answer is {}", 42);
  \endrst
*/
inline std::string format(StringRef format_str, const ArgList &args) {
  Writer w;
  w.write(format_str, args);
  return w.str();
}

inline std::wstring format(WStringRef format_str, const ArgList &args) {
  WWriter w;
  w.write(format_str, args);
  return w.str();
}

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    print(stderr, "Don't {}!", "panic");
  \endrst
 */
void print(std::FILE *f, StringRef format_str, const ArgList &args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    print("Elapsed time: {0:.2f} seconds", 1.23);
  \endrst
 */
inline void print(StringRef format_str, const ArgList &args) {
  print(stdout, format_str, args);
}

/**
  \rst
  Prints formatted data to the stream *os*.

  **Example**::

    print(cerr, "Don't {}!", "panic");
  \endrst
 */
void print(std::ostream &os, StringRef format_str, const ArgList &args);

template <typename Char>
void printf(BasicWriter<Char> &w,
    BasicStringRef<Char> format, const ArgList &args) {
  internal::PrintfFormatter<Char>().format(w, format, args);
}

/**
  \rst
  Formats arguments and returns the result as a string.

  **Example**::

    std::string message = fmt::sprintf("The answer is %d", 42);
  \endrst
*/
inline std::string sprintf(StringRef format, const ArgList &args) {
  Writer w;
  printf(w, format, args);
  return w.str();
}

/**
  \rst
  Prints formatted data to the file *f*.

  **Example**::

    fmt::fprintf(stderr, "Don't %s!", "panic");
  \endrst
 */
int fprintf(std::FILE *f, StringRef format, const ArgList &args);

/**
  \rst
  Prints formatted data to ``stdout``.

  **Example**::

    fmt::printf("Elapsed time: %.2f seconds", 1.23);
  \endrst
 */
inline int printf(StringRef format, const ArgList &args) {
  return fprintf(stdout, format, args);
}

/**
  Fast integer formatter.
 */
class FormatInt {
 private:
  // Buffer should be large enough to hold all digits (digits10 + 1),
  // a sign and a null character.
  enum {BUFFER_SIZE = std::numeric_limits<ULongLong>::digits10 + 3};
  mutable char buffer_[BUFFER_SIZE];
  char *str_;

  // Formats value in reverse and returns the number of digits.
  char *format_decimal(ULongLong value) {
    char *buffer_end = buffer_ + BUFFER_SIZE - 1;
    while (value >= 100) {
      // Integer division is slow so do it for a group of two digits instead
      // of for every digit. The idea comes from the talk by Alexandrescu
      // "Three Optimization Tips for C++". See speed-test for a comparison.
      unsigned index = (value % 100) * 2;
      value /= 100;
      *--buffer_end = internal::DIGITS[index + 1];
      *--buffer_end = internal::DIGITS[index];
    }
    if (value < 10) {
      *--buffer_end = static_cast<char>('0' + value);
      return buffer_end;
    }
    unsigned index = static_cast<unsigned>(value * 2);
    *--buffer_end = internal::DIGITS[index + 1];
    *--buffer_end = internal::DIGITS[index];
    return buffer_end;
  }

  void FormatSigned(LongLong value) {
    ULongLong abs_value = static_cast<ULongLong>(value);
    bool negative = value < 0;
    if (negative)
      abs_value = 0 - abs_value;
    str_ = format_decimal(abs_value);
    if (negative)
      *--str_ = '-';
  }

 public:
  explicit FormatInt(int value) { FormatSigned(value); }
  explicit FormatInt(long value) { FormatSigned(value); }
  explicit FormatInt(LongLong value) { FormatSigned(value); }
  explicit FormatInt(unsigned value) : str_(format_decimal(value)) {}
  explicit FormatInt(unsigned long value) : str_(format_decimal(value)) {}
  explicit FormatInt(ULongLong value) : str_(format_decimal(value)) {}

  /**
    Returns the number of characters written to the output buffer.
   */
  std::size_t size() const { return buffer_ - str_ + BUFFER_SIZE - 1; }

  /**
    Returns a pointer to the output buffer content. No terminating null
    character is appended.
   */
  const char *data() const { return str_; }

  /**
    Returns a pointer to the output buffer content with terminating null
    character appended.
   */
  const char *c_str() const {
    buffer_[BUFFER_SIZE - 1] = '\0';
    return str_;
  }

  /**
    Returns the content of the output buffer as an `std::string`.
   */
  std::string str() const { return std::string(str_, size()); }
};

// Formats a decimal integer value writing into buffer and returns
// a pointer to the end of the formatted string. This function doesn't
// write a terminating null character.
template <typename T>
inline void format_decimal(char *&buffer, T value) {
  typename internal::IntTraits<T>::MainType abs_value = value;
  if (internal::is_negative(value)) {
    *buffer++ = '-';
    abs_value = 0 - abs_value;
  }
  if (abs_value < 100) {
    if (abs_value < 10) {
      *buffer++ = static_cast<char>('0' + abs_value);
      return;
    }
    unsigned index = static_cast<unsigned>(abs_value * 2);
    *buffer++ = internal::DIGITS[index];
    *buffer++ = internal::DIGITS[index + 1];
    return;
  }
  unsigned num_digits = internal::count_digits(abs_value);
  internal::format_decimal(buffer, abs_value, num_digits);
  buffer += num_digits;
}
}

#if FMT_GCC_VERSION
// Use the system_header pragma to suppress warnings about variadic macros
// because suppressing -Wvariadic-macros with the diagnostic pragma doesn't
// work. It is used at the end because we want to suppress as little warnings
// as possible.
# pragma GCC system_header
#endif

// This is used to work around VC++ bugs in handling variadic macros.
#define FMT_EXPAND(args) args

// Returns the number of arguments.
// Based on https://groups.google.com/forum/#!topic/comp.std.c/d-6Mj5Lko_s.
#define FMT_NARG(...) FMT_NARG_(__VA_ARGS__, FMT_RSEQ_N())
#define FMT_NARG_(...) FMT_EXPAND(FMT_ARG_N(__VA_ARGS__))
#define FMT_ARG_N(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define FMT_RSEQ_N() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0

#define FMT_CONCAT(a, b) a##b
#define FMT_FOR_EACH_(N, f, ...) \
  FMT_EXPAND(FMT_CONCAT(FMT_FOR_EACH, N)(f, __VA_ARGS__))
#define FMT_FOR_EACH(f, ...) \
  FMT_EXPAND(FMT_FOR_EACH_(FMT_NARG(__VA_ARGS__), f, __VA_ARGS__))

#define FMT_ADD_ARG_NAME(type, index) type arg##index
#define FMT_GET_ARG_NAME(type, index) arg##index

#if FMT_USE_VARIADIC_TEMPLATES
# define FMT_VARIADIC_(Char, ReturnType, func, call, ...) \
  template<typename... Args> \
  ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__), \
      const Args & ... args) { \
    using fmt::internal::Arg; \
    const Arg array[fmt::internal::NonZero<sizeof...(Args)>::VALUE] = { \
      fmt::internal::MakeArg<Char>(args)... \
    }; \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), \
      fmt::ArgList(array, sizeof...(Args))); \
  }
#else
// Defines a wrapper for a function taking __VA_ARGS__ arguments
// and n additional arguments of arbitrary types.
# define FMT_WRAP(Char, ReturnType, func, call, n, ...) \
  template <FMT_GEN(n, FMT_MAKE_TEMPLATE_ARG)> \
  inline ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__), \
      FMT_GEN(n, FMT_MAKE_ARG)) { \
    const fmt::internal::Arg args[] = {FMT_GEN(n, FMT_MAKE_REF_##Char)}; \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), \
      fmt::ArgList(args, sizeof(args) / sizeof(*args))); \
  }

# define FMT_VARIADIC_(Char, ReturnType, func, call, ...) \
  inline ReturnType func(FMT_FOR_EACH(FMT_ADD_ARG_NAME, __VA_ARGS__)) { \
    call(FMT_FOR_EACH(FMT_GET_ARG_NAME, __VA_ARGS__), fmt::ArgList()); \
  } \
  FMT_WRAP(Char, ReturnType, func, call, 1, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 2, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 3, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 4, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 5, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 6, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 7, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 8, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 9, __VA_ARGS__) \
  FMT_WRAP(Char, ReturnType, func, call, 10, __VA_ARGS__)
#endif  // FMT_USE_VARIADIC_TEMPLATES

/**
  \rst
  Defines a variadic function with the specified return type, function name
  and argument types passed as variable arguments to this macro.

  **Example**::

    void print_error(const char *file, int line, const char *format,
                     const fmt::ArgList &args) {
      fmt::print("{}: {}: ", file, line);
      fmt::print(format, args);
    }
    FMT_VARIADIC(void, print_error, const char *, int, const char *)

  ``FMT_VARIADIC`` is used for compatibility with legacy C++ compilers that
  don't implement variadic templates. You don't have to use this macro if
  you don't need legacy compiler support and can use variadic templates
  directly::

    template<typename... Args>
    void print_error(const char *file, int line, const char *format,
                     const Args & ... args) {
      fmt::print("{}: {}: ", file, line);
      fmt::print(format, args...);
    }
  \endrst
 */
#define FMT_VARIADIC(ReturnType, func, ...) \
  FMT_VARIADIC_(char, ReturnType, func, return func, __VA_ARGS__)

#define FMT_VARIADIC_W(ReturnType, func, ...) \
  FMT_VARIADIC_(wchar_t, ReturnType, func, return func, __VA_ARGS__)

namespace fmt {
FMT_VARIADIC(std::string, format, StringRef)
FMT_VARIADIC_W(std::wstring, format, WStringRef)
FMT_VARIADIC(void, print, StringRef)
FMT_VARIADIC(void, print, std::FILE *, StringRef)
FMT_VARIADIC(void, print, std::ostream &, StringRef)
FMT_VARIADIC(void, print_colored, Color, StringRef)
FMT_VARIADIC(std::string, sprintf, StringRef)
FMT_VARIADIC(int, printf, StringRef)
FMT_VARIADIC(int, fprintf, std::FILE *, StringRef)
}

#include <string.h>

#include <cctype>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstdarg>

#ifdef _WIN32
# define WIN32_LEAN_AND_MEAN
# ifdef __MINGW32__
#  include <cstring>
# endif
# include <windows.h>
# undef ERROR
#endif

#if _MSC_VER
# pragma warning(push)
# pragma warning(disable: 4127) // conditional expression is constant
# pragma warning(disable: 4006)
#endif

#ifndef _MSC_VER

// Portable version of signbit.
// When compiled in C++11 mode signbit is no longer a macro but a function
// defined in namespace std and the macro is undefined.
inline int getsign(double x) {
#ifdef signbit
  return signbit(x);
#else
  return std::signbit(x);
#endif
}

// Portable version of isinf.
#ifdef isinf
inline int isinfinity(double x) { return isinf(x); }
inline int isinfinity(long double x) { return isinf(x); }
#else
inline int isinfinity(double x) { return std::isinf(x); }
inline int isinfinity(long double x) { return std::isinf(x); }
#endif

#define FMT_SNPRINTF snprintf

#else  // _MSC_VER

inline int getsign(double value) {
  if (value < 0) return 1;
  if (value == value) return 0;
  int dec = 0, sign = 0;
  char buffer[2];  // The buffer size must be >= 2 or _ecvt_s will fail.
  _ecvt_s(buffer, sizeof(buffer), value, 0, &dec, &sign);
  return sign;
}

inline int isinfinity(double x) { return !_finite(x); }

inline int fmt_snprintf(char *buffer, size_t size, const char *format, ...) {
  va_list args;
  va_start(args, format);
  int result = vsnprintf_s(buffer, size, _TRUNCATE, format, args);
  va_end(args);
  return result;
}
#define FMT_SNPRINTF fmt_snprintf

#endif  // _MSC_VER

const char RESET_COLOR[] = "\x1b[0m";

template <typename T>
struct IsLongDouble { enum {VALUE = 0}; };

template <>
struct IsLongDouble<long double> { enum {VALUE = 1}; };

// Checks if a value fits in int - used to avoid warnings about comparing
// signed and unsigned integers.
template <bool IsSigned>
struct IntChecker {
  template <typename T>
  static bool fits_in_int(T value) {
    unsigned max = INT_MAX;
    return value <= max;
  }
};

template <>
struct IntChecker<true> {
  template <typename T>
  static bool fits_in_int(T value) {
    return value >= INT_MIN && value <= INT_MAX;
  }
};

typedef void (*FormatFunc)(fmt::Writer &, int , fmt::StringRef);

inline void report_error(FormatFunc func,
    int error_code, fmt::StringRef message) FMT_NOEXCEPT(true) {
  try {
    fmt::Writer full_message;
    func(full_message, error_code, message); // TODO: make sure this doesn't throw
    std::fwrite(full_message.c_str(), full_message.size(), 1, stderr);
    std::fputc('\n', stderr);
  } catch (...) {}
}

const fmt::internal::Arg DUMMY_ARG = {fmt::internal::Arg::INT, {0}};

// IsZeroInt::visit(arg) returns true iff arg is a zero integer.
class IsZeroInt : public fmt::internal::ArgVisitor<IsZeroInt, bool> {
 public:
  template <typename T>
  bool visit_any_int(T value) { return value == 0; }
};

// Parses an unsigned integer advancing s to the end of the parsed input.
// This function assumes that the first character of s is a digit.
template <typename Char>
int parse_nonnegative_int(
    const Char *&s, const char *&error) FMT_NOEXCEPT(true) {
  assert('0' <= *s && *s <= '9');
  unsigned value = 0;
  do {
    unsigned new_value = value * 10 + (*s++ - '0');
    // Check if value wrapped around.
    value = new_value >= value ? new_value : UINT_MAX;
  } while ('0' <= *s && *s <= '9');
  if (value > INT_MAX) {
    if (!error)
      error = "number is too big in format";
    return 0;
  }
  return value;
}

template <typename Char>
const Char *find_closing_brace(const Char *s, int num_open_braces = 1) {
  for (int n = num_open_braces; *s; ++s) {
    if (*s == '{') {
      ++n;
    } else if (*s == '}') {
      if (--n == 0)
        return s;
    }
  }
  throw fmt::FormatError("unmatched '{' in format");
}

template <typename Char>
void fmt::BasicFormatter<Char>::check_sign(
    const Char *&s, const fmt::internal::Arg &arg) {
  char sign = static_cast<char>(*s);
  if (arg.type > fmt::internal::Arg::LAST_NUMERIC_TYPE) {
    report_error_(s, fmt::format(
      "format specifier '{}' requires numeric argument", sign).c_str());
  }
  if (arg.type == fmt::internal::Arg::UINT || arg.type == fmt::internal::Arg::ULONG_LONG) {
    report_error_(s, fmt::format(
      "format specifier '{}' requires signed argument", sign).c_str());
  }
  ++s;
}

template <typename Char>
typename fmt::BasicWriter<Char>::CharPtr
  fmt::BasicWriter<Char>::fill_padding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill) {
  std::size_t padding = total_size - content_size;
  std::size_t left_padding = padding / 2;
  Char fill_char = static_cast<Char>(fill);
  std::fill_n(buffer, left_padding, fill_char);
  buffer += left_padding;
  CharPtr content = buffer;
  std::fill_n(buffer + content_size, padding - left_padding, fill_char);
  return content;
}

template <typename Char>
template <typename T>
void fmt::BasicWriter<Char>::write_double(T value, const FormatSpec &spec) {
  // Check type.
  char type = spec.type();
  bool upper = false;
  switch (type) {
  case 0:
    type = 'g';
    break;
  case 'e': case 'f': case 'g': case 'a':
    break;
  case 'F':
#ifdef _MSC_VER
    // MSVC's printf doesn't support 'F'.
    type = 'f';
#endif
    // Fall through.
  case 'E': case 'G': case 'A':
    upper = true;
    break;
  default:
    internal::report_unknown_type(type, "double");
    break;
  }

  char sign = 0;
  // Use getsign instead of value < 0 because the latter is always
  // false for NaN.
  if (getsign(static_cast<double>(value))) {
    sign = '-';
    value = -value;
  } else if (spec.flag(SIGN_FLAG)) {
    sign = spec.flag(PLUS_FLAG) ? '+' : ' ';
  }

  if (value != value) {
    // Format NaN ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *nan = upper ? " NAN" : " nan";
    if (!sign) {
      --size;
      ++nan;
    }
    CharPtr out = write_str(nan, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  if (isinfinity(value)) {
    // Format infinity ourselves because sprintf's output is not consistent
    // across platforms.
    std::size_t size = 4;
    const char *inf = upper ? " INF" : " inf";
    if (!sign) {
      --size;
      ++inf;
    }
    CharPtr out = write_str(inf, size, spec);
    if (sign)
      *out = sign;
    return;
  }

  std::size_t offset = buffer_.size();
  unsigned width = spec.width();
  if (sign) {
    buffer_.reserve(buffer_.size() + (std::max)(width, 1u));
    if (width > 0)
      --width;
    ++offset;
  }

  // Build format string.
  enum { MAX_FORMAT_SIZE = 10}; // longest format: %#-*.*Lg
  Char format[MAX_FORMAT_SIZE];
  Char *format_ptr = format;
  *format_ptr++ = '%';
  unsigned width_for_sprintf = width;
  if (spec.flag(HASH_FLAG))
    *format_ptr++ = '#';
  if (spec.align() == ALIGN_CENTER) {
    width_for_sprintf = 0;
  } else {
    if (spec.align() == ALIGN_LEFT)
      *format_ptr++ = '-';
    if (width != 0)
      *format_ptr++ = '*';
  }
  if (spec.precision() >= 0) {
    *format_ptr++ = '.';
    *format_ptr++ = '*';
  }
  if (IsLongDouble<T>::VALUE)
    *format_ptr++ = 'L';
  *format_ptr++ = type;
  *format_ptr = '\0';

  // Format using snprintf.
  Char fill = static_cast<Char>(spec.fill());
  for (;;) {
    std::size_t size = buffer_.capacity() - offset;
#if _MSC_VER
    // MSVC's vsnprintf_s doesn't work with zero size, so reserve
    // space for at least one extra character to make the size non-zero.
    // Note that the buffer's capacity will increase by more than 1.
    if (size == 0) {
      buffer_.reserve(offset + 1);
      size = buffer_.capacity() - offset;
    }
#endif
    Char *start = &buffer_[offset];
    int n = internal::CharTraits<Char>::format_float(
        start, size, format, width_for_sprintf, spec.precision(), value);
    if (n >= 0 && offset + n < buffer_.capacity()) {
      if (sign) {
        if ((spec.align() != ALIGN_RIGHT && spec.align() != ALIGN_DEFAULT) ||
            *start != ' ') {
          *(start - 1) = sign;
          sign = 0;
        } else {
          *(start - 1) = fill;
        }
        ++n;
      }
      if (spec.align() == ALIGN_CENTER &&
          spec.width() > static_cast<unsigned>(n)) {
        unsigned width = spec.width();
        CharPtr p = grow_buffer(width);
        std::copy(p, p + n, p + (width - n) / 2);
        fill_padding(p, spec.width(), n, fill);
        return;
      }
      if (spec.fill() != ' ' || sign) {
        while (*start == ' ')
          *start++ = fill;
        if (sign)
          *(start - 1) = sign;
      }
      grow_buffer(n);
      return;
    }
    // If n is negative we ask to increase the capacity by at least 1,
    // but as std::vector, the buffer grows exponentially.
    buffer_.reserve(n >= 0 ? offset + n + 1 : buffer_.capacity() + 1);
  }
}

template <typename Char>
template <typename StrChar>
void fmt::BasicWriter<Char>::write_str(
    const fmt::internal::Arg::StringValue<StrChar> &str, const FormatSpec &spec) {
  // Check if StrChar is convertible to Char.
  internal::CharTraits<Char>::convert(StrChar());
  if (spec.type_ && spec.type_ != 's')
    internal::report_unknown_type(spec.type_, "string");
  const StrChar *s = str.value;
  std::size_t size = str.size;
  if (size == 0) {
    if (!s)
      throw FormatError("string pointer is null");
    if (*s)
      size = std::char_traits<StrChar>::length(s);
  }
  write_str(s, size, spec);
}

template <typename Char>
inline const fmt::internal::Arg
    &fmt::BasicFormatter<Char>::parse_arg_index(const Char *&s) {
  unsigned arg_index = 0;
  if (*s < '0' || *s > '9') {
    if (*s != '}' && *s != ':')
      report_error_(s, "invalid argument index in format string");
    const fmt::internal::Arg &arg = next_arg();
    if (error_)
      report_error_(s, error_);
    return arg;
  }
  if (next_arg_index_ > 0) {
    report_error_(s,
        "cannot switch from automatic to manual argument indexing");
  }
  next_arg_index_ = -1;
  arg_index = parse_nonnegative_int(s, error_);
  if (error_)
    report_error_(s, error_); // TODO: don't use report_error_
  if (arg_index >= args_.size())
    report_error_(s, "argument index is out of range in format");
  return args_[arg_index];
}

// Checks if an argument is a valid printf width specifier and sets
// left alignment if it is negative.
class WidthHandler : public fmt::internal::ArgVisitor<WidthHandler, unsigned> {
 private:
  fmt::FormatSpec &spec_;

 public:
  explicit WidthHandler(fmt::FormatSpec &spec) : spec_(spec) {}

  unsigned visit_unhandled_arg() {
    throw fmt::FormatError("width is not integer");
  }

  template <typename T>
  unsigned visit_any_int(T value) {
    typedef typename fmt::internal::IntTraits<T>::MainType UnsignedType;
    UnsignedType width = value;
    if (fmt::internal::is_negative(value)) {
      spec_.align_ = fmt::ALIGN_LEFT;
      width = 0 - width;
    }
    if (width > INT_MAX)
      throw fmt::FormatError("number is too big in format");
    return static_cast<unsigned>(width);
  }
};

class PrecisionHandler :
    public fmt::internal::ArgVisitor<PrecisionHandler, int> {
 public:
  unsigned visit_unhandled_arg() {
    throw fmt::FormatError("precision is not integer");
  }

  template <typename T>
  int visit_any_int(T value) {
    if (!IntChecker<std::numeric_limits<T>::is_signed>::fits_in_int(value))
      throw fmt::FormatError("number is too big in format");
    return static_cast<int>(value);
  }
};

// Converts an integer argument to type T.
template <typename T>
class ArgConverter : public fmt::internal::ArgVisitor<ArgConverter<T>, void> {
 private:
  fmt::internal::Arg &arg_;
  wchar_t type_;

 public:
  ArgConverter(fmt::internal::Arg &arg, wchar_t type)
    : arg_(arg), type_(type) {}

  template <typename U>
  void visit_any_int(U value) {
    bool is_signed = type_ == 'd' || type_ == 'i';
    using fmt::internal::Arg;
    if (sizeof(T) <= sizeof(int)) {
      if (is_signed) {
        arg_.type = Arg::INT;
        arg_.int_value = static_cast<int>(static_cast<T>(value));
      } else {
        arg_.type = Arg::UINT;
        arg_.uint_value = static_cast<unsigned>(
            static_cast<typename fmt::internal::MakeUnsigned<T>::Type>(value));
      }
    } else {
      if (is_signed) {
        arg_.type = Arg::LONG_LONG;
        arg_.long_long_value =
            static_cast<typename fmt::internal::MakeUnsigned<U>::Type>(value);
      } else {
        arg_.type = Arg::ULONG_LONG;
        arg_.ulong_long_value =
            static_cast<typename fmt::internal::MakeUnsigned<U>::Type>(value);
      }
    }
  }
};

// Converts an integer argument to char.
class CharConverter : public fmt::internal::ArgVisitor<CharConverter, void> {
 private:
  fmt::internal::Arg &arg_;

 public:
  explicit CharConverter(fmt::internal::Arg &arg) : arg_(arg) {}

  template <typename T>
  void visit_any_int(T value) {
    arg_.type = fmt::internal::Arg::CHAR;
    arg_.int_value = static_cast<char>(value);
  }
};

// This function template is used to prevent compile errors when handling
// incompatible string arguments, e.g. handling a wide string in a narrow
// string formatter.
template <typename Char>
fmt::internal::Arg::StringValue<Char> ignore_incompatible_str(fmt::internal::Arg::StringValue<wchar_t>);

template <>
inline fmt::internal::Arg::StringValue<char> ignore_incompatible_str(
    fmt::internal::Arg::StringValue<wchar_t>) { return fmt::internal::Arg::StringValue<char>(); }

template <>
inline fmt::internal::Arg::StringValue<wchar_t> ignore_incompatible_str(
    fmt::internal::Arg::StringValue<wchar_t> s) { return s; }

inline void fmt::SystemError::init(
    int error_code, StringRef format_str, const ArgList &args) {
  error_code_ = error_code;
  Writer w;
  internal::format_system_error(w, error_code, format(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(w.str());
}

template <typename T>
int fmt::internal::CharTraits<char>::format_float(
    char *buffer, std::size_t size, const char *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        FMT_SNPRINTF(buffer, size, format, value) :
        FMT_SNPRINTF(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      FMT_SNPRINTF(buffer, size, format, width, value) :
      FMT_SNPRINTF(buffer, size, format, width, precision, value);
}

template <typename T>
int fmt::internal::CharTraits<wchar_t>::format_float(
    wchar_t *buffer, std::size_t size, const wchar_t *format,
    unsigned width, int precision, T value) {
  if (width == 0) {
    return precision < 0 ?
        swprintf(buffer, size, format, value) :
        swprintf(buffer, size, format, precision, value);
  }
  return precision < 0 ?
      swprintf(buffer, size, format, width, value) :
      swprintf(buffer, size, format, width, precision, value);
}

extern const char fmt::internal::DIGITS[];

#define FMT_POWERS_OF_10(factor) \
  factor * 10, \
  factor * 100, \
  factor * 1000, \
  factor * 10000, \
  factor * 100000, \
  factor * 1000000, \
  factor * 10000000, \
  factor * 100000000, \
  factor * 1000000000

inline void fmt::internal::report_unknown_type(char code, const char *type) {
  if (std::isprint(static_cast<unsigned char>(code))) {
    throw fmt::FormatError(
        fmt::format("unknown format code '{}' for {}", code, type));
  }
  throw fmt::FormatError(
      fmt::format("unknown format code '\\x{:02x}' for {}",
        static_cast<unsigned>(code), type));
}

#ifdef _WIN32

inline fmt::internal::UTF8ToUTF16::UTF8ToUTF16(fmt::StringRef s) {
  int length = MultiByteToWideChar(
      CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, 0, 0);
  static const char ERROR[] = "cannot convert string from UTF-8 to UTF-16";
  if (length == 0)
    throw WindowsError(GetLastError(), ERROR);
  buffer_.resize(length);
  length = MultiByteToWideChar(
    CP_UTF8, MB_ERR_INVALID_CHARS, s.c_str(), -1, &buffer_[0], length);
  if (length == 0)
    throw WindowsError(GetLastError(), ERROR);
}

inline fmt::internal::UTF16ToUTF8::UTF16ToUTF8(fmt::WStringRef s) {
  if (int error_code = convert(s)) {
    throw WindowsError(error_code,
        "cannot convert string from UTF-16 to UTF-8");
  }
}

inline int fmt::internal::UTF16ToUTF8::convert(fmt::WStringRef s) {
  int length = WideCharToMultiByte(CP_UTF8, 0, s.c_str(), -1, 0, 0, 0, 0);
  if (length == 0)
    return GetLastError();
  buffer_.resize(length);
  length = WideCharToMultiByte(
    CP_UTF8, 0, s.c_str(), -1, &buffer_[0], length, 0, 0);
  if (length == 0)
    return GetLastError();
  return 0;
}

inline void fmt::WindowsError::init(
    int error_code, StringRef format_str, const ArgList &args) {
  error_code_ = error_code;
  Writer w;
  internal::format_windows_error(w, error_code, format(format_str, args));
  std::runtime_error &base = *this;
  base = std::runtime_error(w.str());
}

#endif

inline int fmt::internal::safe_strerror(
    int error_code, char *&buffer, std::size_t buffer_size) FMT_NOEXCEPT(true) {
  assert(buffer != 0 && buffer_size != 0);
  int result = 0;
#ifdef _GNU_SOURCE
  char *message = strerror_r(error_code, buffer, buffer_size);
  // If the buffer is full then the message is probably truncated.
  if (message == buffer && strlen(buffer) == buffer_size - 1)
    result = ERANGE;
  buffer = message;
#elif __MINGW32__
  errno = 0;
  (void)buffer_size;
  buffer = strerror(error_code);
  result = errno;
#elif _WIN32
  result = strerror_s(buffer, buffer_size, error_code);
  // If the buffer is full then the message is probably truncated.
  if (result == 0 && std::strlen(buffer) == buffer_size - 1)
    result = ERANGE;
#else
  result = strerror_r(error_code, buffer, buffer_size);
  if (result == -1)
    result = errno;  // glibc versions before 2.13 return result in errno.
#endif
  return result;
}

inline void fmt::internal::format_system_error(
    fmt::Writer &out, int error_code, fmt::StringRef message) {
  Array<char, INLINE_BUFFER_SIZE> buffer;
  buffer.resize(INLINE_BUFFER_SIZE);
  char *system_message = 0;
  for (;;) {
    system_message = &buffer[0];
    int result = safe_strerror(error_code, system_message, buffer.size());
    if (result == 0)
      break;
    if (result != ERANGE) {
      // Can't get error message, report error code instead.
      out << message << ": error code = " << error_code;
      return;
    }
    buffer.resize(buffer.size() * 2);
  }
  out << message << ": " << system_message;
}

#ifdef _WIN32
inline void fmt::internal::format_windows_error(
    fmt::Writer &out, int error_code, fmt::StringRef message) {
  class String {
   private:
    LPWSTR str_;

   public:
    String() : str_() {}
    ~String() { LocalFree(str_); }
    LPWSTR *ptr() { return &str_; }
    LPCWSTR c_str() const { return str_; }
  };
  String system_message;
  if (FormatMessageW(FORMAT_MESSAGE_ALLOCATE_BUFFER |
      FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0,
      error_code, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
      reinterpret_cast<LPWSTR>(system_message.ptr()), 0, 0)) {
    UTF16ToUTF8 utf8_message;
    if (!utf8_message.convert(system_message.c_str())) {
      out << message << ": " << utf8_message;
      return;
    }
  }
  // Can't get error message, report error code instead.
  out << message << ": error code = " << error_code;
}
#endif

// An argument formatter.
template <typename Char>
class fmt::internal::ArgFormatter :
    public fmt::internal::ArgVisitor<fmt::internal::ArgFormatter<Char>, void> {
 private:
  fmt::BasicFormatter<Char> &formatter_;
  fmt::BasicWriter<Char> &writer_;
  fmt::FormatSpec &spec_;
  const Char *format_;

 public:
  ArgFormatter(
      fmt::BasicFormatter<Char> &f,fmt::FormatSpec &s, const Char *fmt)
  : formatter_(f), writer_(f.writer()), spec_(s), format_(fmt) {}

  template <typename T>
  void visit_any_int(T value) { writer_.write_int(value, spec_); }

  template <typename T>
  void visit_any_double(T value) { writer_.write_double(value, spec_); }

  void visit_char(int value) {
    if (spec_.type_ && spec_.type_ != 'c') {
      spec_.flags_ |= CHAR_FLAG;
      writer_.write_int(value, spec_);
      return;
    }
    if (spec_.align_ == ALIGN_NUMERIC || spec_.flags_ != 0)
      throw FormatError("invalid format specifier for char");
    typedef typename fmt::BasicWriter<Char>::CharPtr CharPtr;
    CharPtr out = CharPtr();
    if (spec_.width_ > 1) {
      Char fill = static_cast<Char>(spec_.fill());
      out = writer_.grow_buffer(spec_.width_);
      if (spec_.align_ == fmt::ALIGN_RIGHT) {
        std::fill_n(out, spec_.width_ - 1, fill);
        out += spec_.width_ - 1;
      } else if (spec_.align_ == fmt::ALIGN_CENTER) {
        out = writer_.fill_padding(out, spec_.width_, 1, fill);
      } else {
        std::fill_n(out + 1, spec_.width_ - 1, fill);
      }
    } else {
      out = writer_.grow_buffer(1);
    }
    *out = static_cast<Char>(value);
  }

  void visit_string(Arg::StringValue<char> value) {
    writer_.write_str(value, spec_);
  }
  void visit_wstring(Arg::StringValue<wchar_t> value) {
    writer_.write_str(ignore_incompatible_str<Char>(value), spec_);
  }

  void visit_pointer(const void *value) {
    if (spec_.type_ && spec_.type_ != 'p')
      fmt::internal::report_unknown_type(spec_.type_, "pointer");
    spec_.flags_ = fmt::HASH_FLAG;
    spec_.type_ = 'x';
    writer_.write_int(reinterpret_cast<uintptr_t>(value), spec_);
  }

  void visit_custom(Arg::CustomValue c) {
    c.format(&formatter_, c.value, format_);
  }
};

template <typename Char>
void fmt::internal::FormatErrorReporter<Char>::operator()(
        const Char *s, fmt::StringRef message) const {
  if (find_closing_brace(s, num_open_braces))
    throw fmt::FormatError(message);
}

template <typename Char>
void fmt::internal::PrintfFormatter<Char>::parse_flags(
    FormatSpec &spec, const Char *&s) {
  for (;;) {
    switch (*s++) {
      case '-':
        spec.align_ = ALIGN_LEFT;
        break;
      case '+':
        spec.flags_ |= SIGN_FLAG | PLUS_FLAG;
        break;
      case '0':
        spec.fill_ = '0';
        break;
      case ' ':
        spec.flags_ |= SIGN_FLAG;
        break;
      case '#':
        spec.flags_ |= HASH_FLAG;
        break;
      default:
        --s;
        return;
    }
  }
}

template <typename Char>
unsigned fmt::internal::PrintfFormatter<Char>::parse_header(
  const Char *&s, FormatSpec &spec) {
  unsigned arg_index = UINT_MAX;
  Char c = *s;
  if (c >= '0' && c <= '9') {
    // Parse an argument index (if followed by '$') or a width possibly
    // preceded with '0' flag(s).
    unsigned value = parse_nonnegative_int(s, error_);
    if (*s == '$') {  // value is an argument index
      ++s;
      arg_index = value;
    } else {
      if (c == '0')
        spec.fill_ = '0';
      if (value != 0) {
        // Nonzero value means that we parsed width and don't need to
        // parse it or flags again, so return now.
        spec.width_ = value;
        return arg_index;
      }
    }
  }
  parse_flags(spec, s);
  // Parse width.
  if (*s >= '0' && *s <= '9') {
    spec.width_ = parse_nonnegative_int(s, error_);
  } else if (*s == '*') {
    ++s;
    spec.width_ = WidthHandler(spec).visit(handle_arg_index(UINT_MAX));
  }
  return arg_index;
}

template <typename Char>
void fmt::internal::PrintfFormatter<Char>::format(
    BasicWriter<Char> &writer, BasicStringRef<Char> format,
    const ArgList &args) {
  const Char *start = format.c_str();
  args_ = args;
  next_arg_index_ = 0;
  const Char *s = start;
  while (*s) {
    Char c = *s++;
    if (c != '%') continue;
    if (*s == c) {
      write(writer, start, s);
      start = ++s;
      continue;
    }
    write(writer, start, s - 1);

    FormatSpec spec;
    spec.align_ = ALIGN_RIGHT;

    // Reporting errors is delayed till the format specification is
    // completely parsed. This is done to avoid potentially confusing
    // error messages for incomplete format strings. For example, in
    //   sprintf("%2$", 42);
    // the format specification is incomplete. In a naive approach we
    // would parse 2 as an argument index and report an error that the
    // index is out of range which would be rather confusing if the
    // use meant "%2d$" rather than "%2$d". If we delay an error, the
    // user will get an error that the format string is invalid which
    // is OK for both cases.

    // Parse argument index, flags and width.
    unsigned arg_index = parse_header(s, spec);

    // Parse precision.
    if (*s == '.') {
      ++s;
      if ('0' <= *s && *s <= '9') {
        spec.precision_ = parse_nonnegative_int(s, error_);
      } else if (*s == '*') {
        ++s;
        spec.precision_ = PrecisionHandler().visit(handle_arg_index(UINT_MAX));
      }
    }

    Arg arg = handle_arg_index(arg_index);
    if (spec.flag(HASH_FLAG) && IsZeroInt().visit(arg))
      spec.flags_ &= ~HASH_FLAG;
    if (spec.fill_ == '0') {
      if (arg.type <= Arg::LAST_NUMERIC_TYPE)
        spec.align_ = ALIGN_NUMERIC;
      else
        spec.fill_ = ' ';  // Ignore '0' flag for non-numeric types.
    }

    // Parse length and convert the argument to the required type.
    switch (*s++) {
    case 'h':
      if (*s == 'h')
        ArgConverter<signed char>(arg, *++s).visit(arg);
      else
        ArgConverter<short>(arg, *s).visit(arg);
      break;
    case 'l':
      if (*s == 'l')
        ArgConverter<fmt::LongLong>(arg, *++s).visit(arg);
      else
        ArgConverter<long>(arg, *s).visit(arg);
      break;
    case 'j':
      ArgConverter<intmax_t>(arg, *s).visit(arg);
      break;
    case 'z':
      ArgConverter<size_t>(arg, *s).visit(arg);
      break;
    case 't':
      ArgConverter<ptrdiff_t>(arg, *s).visit(arg);
      break;
    case 'L':
      // printf produces garbage when 'L' is omitted for long double, no
      // need to do the same.
      break;
    default:
      --s;
      ArgConverter<int>(arg, *s).visit(arg);
    }

    // Parse type.
    if (!*s)
      throw FormatError("invalid format string");
    if (error_)
      throw FormatError(error_);
    spec.type_ = static_cast<char>(*s++);
    if (arg.type <= Arg::LAST_INTEGER_TYPE) {
      // Normalize type.
      switch (spec.type_) {
      case 'i': case 'u':
        spec.type_ = 'd';
        break;
      case 'c':
        // TODO: handle wchar_t
        CharConverter(arg).visit(arg);
        break;
      }
    }

    start = s;

    // Format argument.
    switch (arg.type) {
    case Arg::INT:
      writer.write_int(arg.int_value, spec);
      break;
    case Arg::UINT:
      writer.write_int(arg.uint_value, spec);
      break;
    case Arg::LONG_LONG:
      writer.write_int(arg.long_long_value, spec);
      break;
    case Arg::ULONG_LONG:
      writer.write_int(arg.ulong_long_value, spec);
      break;
    case Arg::CHAR: {
      if (spec.type_ && spec.type_ != 'c')
        writer.write_int(arg.int_value, spec);
      typedef typename BasicWriter<Char>::CharPtr CharPtr;
      CharPtr out = CharPtr();
      if (spec.width_ > 1) {
        Char fill = ' ';
        out = writer.grow_buffer(spec.width_);
        if (spec.align_ != ALIGN_LEFT) {
          std::fill_n(out, spec.width_ - 1, fill);
          out += spec.width_ - 1;
        } else {
          std::fill_n(out + 1, spec.width_ - 1, fill);
        }
      } else {
        out = writer.grow_buffer(1);
      }
      *out = static_cast<Char>(arg.int_value);
      break;
    }
    case Arg::DOUBLE:
      writer.write_double(arg.double_value, spec);
      break;
    case Arg::LONG_DOUBLE:
      writer.write_double(arg.long_double_value, spec);
      break;
    case Arg::STRING:
      writer.write_str(arg.string, spec);
      break;
    case Arg::WSTRING:
      writer.write_str(ignore_incompatible_str<Char>(arg.wstring), spec);
      break;
    case Arg::POINTER:
      if (spec.type_ && spec.type_ != 'p')
        internal::report_unknown_type(spec.type_, "pointer");
      spec.flags_= HASH_FLAG;
      spec.type_ = 'x';
      writer.write_int(reinterpret_cast<uintptr_t>(arg.pointer_value), spec);
      break;
    case Arg::CUSTOM:
      if (spec.type_)
        internal::report_unknown_type(spec.type_, "object");
      arg.custom.format(&writer, arg.custom.value, "s");
      break;
    default:
      assert(false);
      break;
    }
  }
  write(writer, start, s);
}

template <typename Char>
const Char *fmt::BasicFormatter<Char>::format(
    const Char *format_str, const fmt::internal::Arg &arg) {
  const Char *s = format_str;
  const char *error = 0;
  FormatSpec spec;
  if (*s == ':') {
    if (arg.type == fmt::internal::Arg::CUSTOM) {
      arg.custom.format(this, arg.custom.value, s);
      return find_closing_brace(s) + 1;
    }
    ++s;
    // Parse fill and alignment.
    if (Char c = *s) {
      const Char *p = s + 1;
      spec.align_ = ALIGN_DEFAULT;
      do {
        switch (*p) {
          case '<':
            spec.align_ = ALIGN_LEFT;
            break;
          case '>':
            spec.align_ = ALIGN_RIGHT;
            break;
          case '=':
            spec.align_ = ALIGN_NUMERIC;
            break;
          case '^':
            spec.align_ = ALIGN_CENTER;
            break;
        }
        if (spec.align_ != ALIGN_DEFAULT) {
          if (p != s) {
            if (c == '}') break;
            if (c == '{')
              report_error_(s, "invalid fill character '{'");
            s += 2;
            spec.fill_ = c;
          } else ++s;
          if (spec.align_ == ALIGN_NUMERIC && arg.type > fmt::internal::Arg::LAST_NUMERIC_TYPE)
            report_error_(s, "format specifier '=' requires numeric argument");
          break;
        }
      } while (--p >= s);
    }

    // Parse sign.
    switch (*s) {
      case '+':
        check_sign(s, arg);
        spec.flags_ |= SIGN_FLAG | PLUS_FLAG;
        break;
      case '-':
        check_sign(s, arg);
        spec.flags_ |= MINUS_FLAG;
        break;
      case ' ':
        check_sign(s, arg);
        spec.flags_ |= SIGN_FLAG;
        break;
    }

    if (*s == '#') {
      if (arg.type > fmt::internal::Arg::LAST_NUMERIC_TYPE)
        report_error_(s, "format specifier '#' requires numeric argument");
      spec.flags_ |= HASH_FLAG;
      ++s;
    }

    // Parse width and zero flag.
    if ('0' <= *s && *s <= '9') {
      if (*s == '0') {
        if (arg.type > fmt::internal::Arg::LAST_NUMERIC_TYPE)
          report_error_(s, "format specifier '0' requires numeric argument");
        spec.align_ = ALIGN_NUMERIC;
        spec.fill_ = '0';
      }
      // Zero may be parsed again as a part of the width, but it is simpler
      // and more efficient than checking if the next char is a digit.
      spec.width_ = parse_nonnegative_int(s, error);
      if (error)
        report_error_(s, error);
    }

    // Parse precision.
    if (*s == '.') {
      ++s;
      spec.precision_ = 0;
      if ('0' <= *s && *s <= '9') {
        spec.precision_ = parse_nonnegative_int(s, error);
        if (error)
          report_error_(s, error);
      } else if (*s == '{') {
        ++s;
        ++report_error_.num_open_braces;
        const fmt::internal::Arg &precision_arg = parse_arg_index(s);
        fmt::ULongLong value = 0;
        switch (precision_arg.type) {
          case fmt::internal::Arg::INT:
            if (precision_arg.int_value < 0)
              report_error_(s, "negative precision in format");
            value = precision_arg.int_value;
            break;
          case fmt::internal::Arg::UINT:
            value = precision_arg.uint_value;
            break;
          case fmt::internal::Arg::LONG_LONG:
            if (precision_arg.long_long_value < 0)
              report_error_(s, "negative precision in format");
            value = precision_arg.long_long_value;
            break;
          case fmt::internal::Arg::ULONG_LONG:
            value = precision_arg.ulong_long_value;
            break;
          default:
            report_error_(s, "precision is not integer");
        }
        if (value > INT_MAX)
          report_error_(s, "number is too big in format");
        spec.precision_ = static_cast<int>(value);
        if (*s++ != '}')
          throw FormatError("unmatched '{' in format");
        --report_error_.num_open_braces;
      } else {
        report_error_(s, "missing precision in format");
      }
      if (arg.type != fmt::internal::Arg::DOUBLE && arg.type != fmt::internal::Arg::LONG_DOUBLE) {
        report_error_(s,
            "precision specifier requires floating-point argument");
      }
    }

    // Parse type.
    if (*s != '}' && *s)
      spec.type_ = static_cast<char>(*s++);
  }

  if (*s++ != '}')
    throw FormatError("unmatched '{' in format");
  start_ = s;

  // Format argument.
  internal::ArgFormatter<Char>(*this, spec, s - 1).visit(arg);
  return s;
}

template <typename Char>
void fmt::BasicFormatter<Char>::format(
    BasicStringRef<Char> format_str, const ArgList &args) {
  const Char *s = start_ = format_str.c_str();
  args_ = args;
  next_arg_index_ = 0;
  while (*s) {
    Char c = *s++;
    if (c != '{' && c != '}') continue;
    if (*s == c) {
      write(writer_, start_, s);
      start_ = ++s;
      continue;
    }
    if (c == '}')
      throw FormatError("unmatched '}' in format");
    report_error_.num_open_braces = 1;
    write(writer_, start_, s - 1);
    fmt::internal::Arg arg = parse_arg_index(s);
    s = format(s, arg);
  }
  write(writer_, start_, s);
}

inline void fmt::report_system_error(
    int error_code, fmt::StringRef message) FMT_NOEXCEPT(true) {
  // FIXME: format_system_error may throw
  report_error(internal::format_system_error, error_code, message);
}

#ifdef _WIN32
inline void fmt::report_windows_error(
    int error_code, fmt::StringRef message) FMT_NOEXCEPT(true) {
  // FIXME: format_windows_error may throw
  report_error(internal::format_windows_error, error_code, message);
}
#endif

inline void fmt::print(std::FILE *f, StringRef format_str, const ArgList &args) {
  Writer w;
  w.write(format_str, args);
  std::fwrite(w.data(), 1, w.size(), f);
}

inline void fmt::print(std::ostream &os, StringRef format_str, const ArgList &args) {
  Writer w;
  w.write(format_str, args);
  os.write(w.data(), w.size());
}

inline void fmt::print_colored(Color c, StringRef format, const ArgList &args) {
  char escape[] = "\x1b[30m";
  escape[3] = '0' + static_cast<char>(c);
  std::fputs(escape, stdout);
  print(format, args);
  std::fputs(RESET_COLOR, stdout);
}

inline int fmt::fprintf(std::FILE *f, StringRef format, const ArgList &args) {
  Writer w;
  printf(w, format, args);
  return std::fwrite(w.data(), 1, w.size(), f);
}

// Explicit instantiations for char.

template fmt::BasicWriter<char>::CharPtr
  fmt::BasicWriter<char>::fill_padding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill);

template void fmt::BasicFormatter<char>::format(
  BasicStringRef<char> format, const ArgList &args);

template void fmt::internal::PrintfFormatter<char>::format(
  BasicWriter<char> &writer, BasicStringRef<char> format, const ArgList &args);

// Explicit instantiations for wchar_t.

template fmt::BasicWriter<wchar_t>::CharPtr
  fmt::BasicWriter<wchar_t>::fill_padding(CharPtr buffer,
    unsigned total_size, std::size_t content_size, wchar_t fill);

template void fmt::BasicFormatter<wchar_t>::format(
    BasicStringRef<wchar_t> format, const ArgList &args);

template void fmt::internal::PrintfFormatter<wchar_t>::format(
    BasicWriter<wchar_t> &writer, BasicStringRef<wchar_t> format,
    const ArgList &args);

#if _MSC_VER
# pragma warning(pop)
#endif

// Restore warnings.
#if FMT_GCC_VERSION >= 406
# pragma GCC diagnostic pop
#endif

#endif  // FMT_FORMAT_H_
