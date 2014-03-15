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

#if defined(_MSC_VER)
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/ndc.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/threadspecificdata.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

NDC::NDC(const std::string& message)
{
        push(message);
}

NDC::~NDC()
{
        pop();
}


LogString& NDC::getMessage(NDC::DiagnosticContext& ctx) {
    return ctx.first;
}

LogString& NDC::getFullMessage(NDC::DiagnosticContext& ctx) {
    return ctx.second;
}

void NDC::clear()
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        while(!stack.empty()) {
          stack.pop();
        }
        data->recycle();
    }
}

NDC::Stack* NDC::cloneStack()
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if (!stack.empty()) {
            return new Stack(stack);
        }
    }
    return new Stack();
}

void NDC::inherit(NDC::Stack * stack) {
    if (stack != NULL) {
        ThreadSpecificData::inherit(*stack);
        delete stack;
    }
}


bool NDC::get(LogString& dest)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty()) {
                dest.append(getFullMessage(stack.top()));
                return true;
        }
        data->recycle();
    }
    return false;
}

int NDC::getDepth() {
    int size = 0;
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        size = data->getStack().size();
        if (size == 0) {
            data->recycle();
        }
    }
    return size;
}

LogString NDC::pop()
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                LogString value(getMessage(stack.top()));
                stack.pop();
                data->recycle();
                return value;
        }
        data->recycle();
    }
    return LogString();
}

bool NDC::pop(std::string& dst)
{
    bool retval = false;
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(getMessage(stack.top()), dst);
                stack.pop();
                retval = true;
        }
        data->recycle();
    }
    return retval;
}

LogString NDC::peek()
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                return getMessage(stack.top());
        }
        data->recycle();
    }
    return LogString();
}

bool NDC::peek(std::string& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(getMessage(stack.top()), dst);
                return true;
        }
        data->recycle();
    }
    return false;
}

void NDC::pushLS(const LogString& message)
{
    ThreadSpecificData::push(message);
}

void NDC::push(const std::string& message)
{
   LOG4CXX_DECODE_CHAR(msg, message);
   pushLS(msg);
}

void NDC::remove()
{
        clear();
}

bool NDC::empty() {
    bool empty = true;
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        empty = stack.empty();
        if (empty) {
            data->recycle();
        }
    }
    return empty;
}

#if LOG4CXX_WCHAR_T_API
NDC::NDC(const std::wstring& message)
{
        push(message);
}

void NDC::push(const std::wstring& message)
{
   LOG4CXX_DECODE_WCHAR(msg, message);
   pushLS(msg);
}

bool NDC::pop(std::wstring& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(getMessage(stack.top()), dst);
                stack.pop();
                data->recycle();
                return true;
        }
        data->recycle();
    }
    return false;
}

bool NDC::peek(std::wstring& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(getMessage(stack.top()), dst);
                return true;
        }
        data->recycle();
    }
    return false;
}

#endif


#if LOG4CXX_UNICHAR_API
NDC::NDC(const std::basic_string<UniChar>& message)
{
        push(message);
}

void NDC::push(const std::basic_string<UniChar>& message)
{
   LOG4CXX_DECODE_UNICHAR(msg, message);
   pushLS(msg);
}

bool NDC::pop(std::basic_string<UniChar>& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(stack.top().message, dst);
                stack.pop();
                data->recycle();
                return true;
        }
        data->recycle();
    }
    return false;
}

bool NDC::peek(std::basic_string<UniChar>& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                Transcoder::encode(stack.top().message, dst);
                return true;
        }
        data->recycle();
    }
    return false;
}

#endif


#if LOG4CXX_CFSTRING_API
NDC::NDC(const CFStringRef& message)
{
        push(message);
}

void NDC::push(const CFStringRef& message)
{
   LOG4CXX_DECODE_CFSTRING(msg, message);
   pushLS(msg);
}

bool NDC::pop(CFStringRef& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                dst = Transcoder::encode(stack.top().message);
                stack.pop();
                data->recycle();
                return true;
        }
        data->recycle();
    }
    return false;
}

bool NDC::peek(CFStringRef& dst)
{
    ThreadSpecificData* data = ThreadSpecificData::getCurrentData();
    if (data != 0) {
        Stack& stack = data->getStack();
        if(!stack.empty())
        {
                dst = Transcoder::encode(stack.top().message);
                return true;
        }
        data->recycle();
    }
    return false;
}

#endif

