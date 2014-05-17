/*
 * Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <log4cxx/logstring.h>
#include <log4cxx/helpers/transform.h>

using namespace log4cxx;
using namespace log4cxx::helpers;



void Transform::appendEscapingTags(
   LogString& buf, const LogString& input)
{
   //Check if the string is zero length -- if so, return
   //what was sent in.

   if(input.length() == 0 )
   {
      return;
   }
   
   logchar specials[] = { 0x22 /* " */, 0x26 /* & */, 0x3C /* < */, 0x3E /* > */, 0x00 };
   size_t start = 0;
   size_t special = input.find_first_of(specials, start);
   while(special != LogString::npos) {
        if (special > start) {
            buf.append(input, start, special - start);
        }
        switch(input[special]) {
            case 0x22:
            buf.append(LOG4CXX_STR("&quot;"));
            break;
            
            case 0x26:
            buf.append(LOG4CXX_STR("&amp;"));
            break;
            
            case 0x3C:
            buf.append(LOG4CXX_STR("&lt;"));
            break;
            
            case 0x3E:
            buf.append(LOG4CXX_STR("&gt;"));
            break;
            
            default:
            buf.append(1, input[special]);
            break;
        }
        start = special+1;
        if (special < input.size()) {
            special = input.find_first_of(specials, start);
        } else {
            special = LogString::npos;
        }
   }
   
   if (start < input.size()) {
        buf.append(input, start, input.size() - start);
    }
}

void Transform::appendEscapingCDATA(
   LogString& buf, const LogString& input)
{
     static const LogString CDATA_END(LOG4CXX_STR("]]>"));
     static const LogString CDATA_EMBEDED_END(LOG4CXX_STR("]]>]]&gt;<![CDATA["));

     const LogString::size_type CDATA_END_LEN = 3;


   if(input.length() == 0 )
   {
      return;
   }

   LogString::size_type end = input.find(CDATA_END);
   if (end == LogString::npos)
   {
      buf.append(input);
      return;
   }

   LogString::size_type start = 0;
   while (end != LogString::npos)
   {
      buf.append(input, start, end-start);
      buf.append(CDATA_EMBEDED_END);
      start = end + CDATA_END_LEN;
      if (start < input.length())
      {
         end = input.find(CDATA_END, start);
      }
      else
      {
         return;
      }
   }

   buf.append(input, start, input.length() - start);
}

