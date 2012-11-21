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

    std::vector<std::string> split(const std::string& str, const std::string& delimiters) {
        std::vector<std::string> ss;

        Tokenizer tokenizer(str, delimiters);
        while (tokenizer.nextToken()) {
            ss.push_back(tokenizer.getToken());
        }

        return ss;
    }
}

namespace strutil {

    const std::string Tokenizer::DEFAULT_DELIMITERS(" \t\n\r");

    Tokenizer::Tokenizer(const std::string& str)
        : m_String(str), m_Offset(0), m_Delimiters(DEFAULT_DELIMITERS) {}

    Tokenizer::Tokenizer(const std::string& str, const std::string& delimiters)
        : m_String(str), m_Offset(0), m_Delimiters(delimiters) {}

    bool Tokenizer::nextToken() {
        return nextToken(m_Delimiters);
    }

    bool Tokenizer::nextToken(const std::string& delimiters) {
        // find the start charater of the next token.
        size_t i = m_String.find_first_not_of(delimiters, m_Offset);
        if (i == std::string::npos) {
            m_Offset = m_String.length();
            return false;
        }

        // find the end of the token.
        size_t j = m_String.find_first_of(delimiters, i);
        if (j == std::string::npos) {
            m_Token = m_String.substr(i);
            m_Offset = m_String.length();
            return true;
        }

        // to intercept the token and save current position
        m_Token = m_String.substr(i, j - i);
        m_Offset = j;
        return true;
    }

    const std::string Tokenizer::getToken() const {
        return m_Token;
    }

    void Tokenizer::reset() {
        m_Offset = 0;
    }
}
}
