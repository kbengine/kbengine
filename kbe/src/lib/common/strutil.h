/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2017 KBEngine.

KBEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

KBEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.
 
You should have received a copy of the GNU Lesser General Public License
along with KBEngine.  If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef KBE_STRUTIL_H
#define KBE_STRUTIL_H

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

namespace KBEngine{ 
/*---------------------------------------------------------------------------------
	��ƽ̨�ӿڶ���
---------------------------------------------------------------------------------*/
#if defined( unix )

#define kbe_isnan isnan
#define kbe_isinf isinf
#define kbe_snprintf snprintf
#define kbe_vsnprintf vsnprintf
#define kbe_vsnwprintf vsnwprintf
#define kbe_snwprintf swprintf
#define kbe_stricmp strcasecmp
#define kbe_strnicmp strncasecmp
#define kbe_fileno fileno
#define kbe_va_copy va_copy
#else
#define kbe_isnan _isnan
#define kbe_isinf(x) (!_finite(x) && !_isnan(x))
#define kbe_snprintf _snprintf
#define kbe_vsnprintf _vsnprintf
#define kbe_vsnwprintf _vsnwprintf
#define kbe_snwprintf _snwprintf
#define kbe_stricmp _stricmp
#define kbe_strnicmp _strnicmp
#define kbe_fileno _fileno
#define kbe_va_copy( dst, src) dst = src

#define strtoq   _strtoi64
#define strtouq  _strtoui64
#define strtoll  _strtoi64
#define strtoull _strtoui64
#define atoll    _atoi64

#endif // unix

class MemoryStream;

// declaration
namespace strutil {

	std::string &kbe_ltrim(std::string &s);
	std::string &kbe_rtrim(std::string &s);
	std::string kbe_trim(std::string s);

	int kbe_replace(std::string& str,  const std::string& pattern,  const std::string& newpat);
	int kbe_replace(std::wstring& str,  const std::wstring& pattern,  const std::wstring& newpat);

	std::string toLower(const std::string& str);
	std::string toUpper(const std::string& str);

	/*
	 ת��Ϊ��д
	*/
	inline char* str_toupper(char* s)
	{
	  assert(s != NULL);

	  while(*s)
	  {
		*s = toupper((unsigned char)*s);
		 s++;
	  };

	  return s; 
	}

	/*
	 ת��ΪСд
	*/
	inline char* str_tolower(char* s)
	{
	  assert(s != NULL);

	  while(*s)
	  {
		*s = tolower((unsigned char)*s);
		 s++;
	  };

	  return s; 
	}

	template<typename T>
	inline void kbe_split(const std::basic_string<T>& s, T c, std::vector< std::basic_string<T> > &v)
	{
		if(s.size() == 0)
			return;

		typename std::basic_string< T >::size_type i = 0;
		typename std::basic_string< T >::size_type j = s.find(c);

		while(j != std::basic_string<T>::npos)
		{
			std::basic_string<T> buf = s.substr(i, j - i);

			if(buf.size() > 0)
				v.push_back(buf);

			i = ++j; j = s.find(c, j);
		}

		if(j == std::basic_string<T>::npos)
		{
			std::basic_string<T> buf = s.substr(i, s.length() - i);
			if(buf.size() > 0)
				v.push_back(buf);
		}
	}

    std::vector< std::string > kbe_splits(const std::string& s, const std::string& delim, const bool keep_empty = true);

	int bytes2string(unsigned char *pSrc, int nSrcLen, unsigned char *pDst, int nDstMaxLen);
	int string2bytes(unsigned char* szSrc, unsigned char* pDst, int nDstMaxLen);
	  
}

namespace strutil {
	// vector<string>֮�������ʹ�� std::find_if �������Ƿ����ĳ���ַ���
	template<typename T>
	class find_vec_string_exist_handle
	{
	public:
		find_vec_string_exist_handle(std::basic_string< T > str)
		: str_(str) {}

		bool operator()(const std::basic_string< T > &strSrc)
		{
			return strSrc == str_;
		}

		bool operator()(const T* strSrc)
		{
			return strSrc == str_;
		}
	private:
		std::basic_string< T > str_;
	};
}

// Tokenizer class
namespace strutil {
    class Tokenizer {
    public:
        static const std::string DEFAULT_DELIMITERS;
        Tokenizer(const std::string& str);
        Tokenizer(const std::string& str, const std::string& delimiters);

        bool nextToken();
        bool nextToken(const std::string& delimiters);
        const std::string getToken() const;

        /**
        * to reset the tokenizer. After reset it, the tokenizer can get
        * the tokens from the first token.
        */
        void reset();

    protected:
        size_t m_Offset;
        const std::string m_String;
        std::string m_Token;
        std::string m_Delimiters;
    };

}

// utf-8
namespace strutil {
	
	char* wchar2char(const wchar_t* ts, size_t* outlen = NULL);
	void wchar2char(const wchar_t* ts, MemoryStream* pStream);
	wchar_t* char2wchar(const char* cs, size_t* outlen = NULL);

	/*
	int wchar2utf8(const wchar_t* in, int in_len, char* out, int out_max);
	int wchar2utf8(const std::wstring& in, std::string& out);
  
	int utf82wchar(const char* in, int in_len, wchar_t* out, int out_max);
	int utf82wchar(const std::string& in, std::wstring& out);
	*/

	bool utf82wchar(const std::string& utf8str, std::wstring& wstr);

	bool utf82wchar(char const* utf8str, size_t csize, wchar_t* wstr, size_t& wsize);
	inline bool utf82wchar(const std::string& utf8str, wchar_t* wstr, size_t& wsize)
	{
		return utf82wchar(utf8str.c_str(), utf8str.size(), wstr, wsize);
	}

	bool wchar2utf8(const std::wstring& wstr, std::string& utf8str);
	bool wchar2utf8(const wchar_t* wstr, size_t size, std::string& utf8str);

	size_t utf8length(const std::string& utf8str);                    // set string to "" if invalid utf8 sequence
	void utf8truncate(const std::string& utf8str, size_t len);
}

}

#endif // KBE_STRUTIL_H

