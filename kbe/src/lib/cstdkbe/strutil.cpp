/*
This source file is part of KBEngine
For the latest info, see http://www.kbengine.org/

Copyright (c) 2008-2012 KBEngine.

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

#include "cstdkbe.hpp"
#include "strutil.hpp"
#include <algorithm>
#include <limits>
#include <algorithm>
#include <utility>
#include <functional>
#include <cctype>

namespace KBEngine{ 
namespace strutil {

    std::string toLower(const std::string& str) {
        std::string t = str;
        std::transform(t.begin(), t.end(), t.begin(), tolower);
        return t;
    }

    std::string toUpper(const std::string& str) {
        std::string t = str;
        std::transform(t.begin(), t.end(), t.begin(), toupper);
        return t;
    }

	std::string &kbe_ltrim(std::string &s) 
	{
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
		return s;
	}

	std::string &kbe_rtrim(std::string &s) 
	{
		s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
		return s;
	}

	std::string &kbe_trim(std::string &s) 
	{
		return kbe_ltrim(kbe_rtrim(s));
	}

	std::string kbe_trim(std::string s) 
	{
		return kbe_ltrim(kbe_rtrim(s));
	}

	// ×Ö·û´®Ìæ»»
	int kbe_replace(std::string& str,  const std::string& pattern,  const std::string& newpat) 
	{ 
		int count = 0; 
		const size_t nsize = newpat.size(); 
		const size_t psize = pattern.size(); 

		for(size_t pos = str.find(pattern, 0);  
			pos != std::string::npos; 
			pos = str.find(pattern,pos + nsize)) 
		{ 
			str.replace(pos, psize, newpat); 
			count++; 
		} 

		return count; 
	}

	int kbe_replace(std::wstring& str,  const std::wstring& pattern,  const std::wstring& newpat) 
	{ 
		int count = 0; 
		const size_t nsize = newpat.size(); 
		const size_t psize = pattern.size(); 

		for(size_t pos = str.find(pattern, 0);  
			pos != std::string::npos; 
			pos = str.find(pattern,pos + nsize)) 
		{ 
			str.replace(pos, psize, newpat); 
			count++; 
		} 

		return count; 
	}


	std::vector< std::string > kbe_splits(const std::string& s, const std::string& delim, const bool keep_empty) 
	{
		std::vector< std::string > result;

		if (delim.empty()) {
			result.push_back(s);
			return result;
		}

		std::string::const_iterator substart = s.begin(), subend;

		while (true) {
			subend = std::search(substart, s.end(), delim.begin(), delim.end());
			std::string temp(substart, subend);
			if (keep_empty || !temp.empty()) {
				result.push_back(temp);
			}
			if (subend == s.end()) {
				break;
			}
			substart = subend + delim.size();
		}

		return result;
	}

	char* wchar2char(const wchar_t* ts)
	{
		int len = (wcslen(ts) + 1) * 4;
		char* ccattr =(char *)malloc(len);
		memset(ccattr, 0, len);
		wcstombs(ccattr, ts, len);
		return ccattr;
	};

	wchar_t* char2wchar(const char* cs)
	{
		int len = (strlen(cs) + 1) * 4;
		wchar_t* ccattr =(wchar_t *)malloc(len);
		memset(ccattr, 0, len);
		mbstowcs(ccattr, cs, len);
		return ccattr;
	};

	int wchar2utf8(const wchar_t* in, int in_len, char* out, int out_max)   
	{   
	#ifdef WIN32   
		BOOL use_def_char;   
		use_def_char = FALSE;   
		return ::WideCharToMultiByte(CP_UTF8, 0, in,in_len / sizeof(wchar_t), out, out_max, NULL, NULL);   
	#else   
		size_t result;   
		iconv_t env;   
	   
		env = iconv_open("UTF8", "WCHAR_T");   
		result = iconv(env,(char**)&in,(size_t*)&in_len,(char**)&out,(size_t*)&out_max);        
		iconv_close(env);   
		return (int) result;   
	#endif   
	}   
	   
	int wchar2utf8(const std::wstring& in, std::string& out)   
	{   
		int len = in.length() + 1;   
		int result;   

		char* pBuffer = new char[len * 4];   

		memset(pBuffer,0,len * 4);               

		result = wchar2utf8(in.c_str(), in.length() * sizeof(wchar_t), pBuffer,len * 4);   

		if(result >= 0)   
		{   
			out = pBuffer;   
		}   
		else   
		{   
			out = "";   
		}   

		delete[] pBuffer;   
		return result;   
	}   
	   
	int utf82wchar(const char* in, int in_len, wchar_t* out, int out_max)   
	{   
	#ifdef WIN32   
		return ::MultiByteToWideChar(CP_UTF8, 0, in, in_len, out, out_max);   
	#else   
		size_t result;   
		iconv_t env;   
		env = iconv_open("WCHAR_T", "UTF8");   
		result = iconv(env,(char**)&in, (size_t*)&in_len, (char**)&out,(size_t*)&out_max);   
		iconv_close(env);   
		return (int) result;   
	#endif   
	}   
	   
	int utf82wchar(const std::string& in, std::wstring& out)   
	{   
		int len = in.length() + 1;   
		int result;   
	 
		wchar_t* pBuffer = new wchar_t[len];   
		memset(pBuffer,0,len * sizeof(wchar_t));   
		result = utf82wchar(in.c_str(), in.length(), pBuffer, len*sizeof(wchar_t));   

		if(result >= 0)   
		{   
			out = pBuffer;   
		}   
		else   
		{   
			out.clear();         
		}   

		delete[] pBuffer;   
		return result;   
	}   
}

}
