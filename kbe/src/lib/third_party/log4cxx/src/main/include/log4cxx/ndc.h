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

#ifndef _LOG4CXX_NDC_H
#define _LOG4CXX_NDC_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/log4cxx.h>
#include <log4cxx/logstring.h>
#include <stack>

namespace log4cxx
{
        /**
        the ndc class implements <i>nested diagnostic contexts</i> as
        defined by neil harrison in the article "patterns for logging
        diagnostic messages" part of the book "<i>pattern languages of
        program design 3</i>" edited by martin et al.

        <p>a nested diagnostic context, or ndc in short, is an instrument
        to distinguish interleaved log output from different sources. log
        output is typically interleaved when a server handles multiple
        clients near-simultaneously.

        <p>interleaved log output can still be meaningful if each log entry
        from different contexts had a distinctive stamp. this is where ndcs
        come into play.

        <p><em><b>note that ndcs are managed on a per thread
        basis</b></em>. ndc operations such as #push,
        #pop, #clear and #getDepth
        affect the ndc of the <em>current</em> thread only. ndcs of other
        threads remain unaffected.

        <p>for example, a servlet can build a per client request ndc
        consisting the clients host name and other information contained in
        the the request. <em>cookies</em> are another source of distinctive
        information. to build an ndc one uses the #push
        operation. simply put,

        <p><ul>
         <li>contexts can be nested.

         <p><li>when entering a context, call <code>ndc.push</code>. as a
         side effect, if there is no nested diagnostic context for the
         current thread, this method will create it.

         <p><li>when leaving a context, call <code>ndc.pop</code>.

         <p><li><b>when exiting a thread make sure to call #remove
         </b>.
        </ul>

        <p>there is no penalty for forgetting to match each
        <code>push</code> operation with a corresponding <code>pop</code>,
        except the obvious mismatch between the real application context
        and the context set in the ndc.

        <p>if configured to do so, PatternLayout and
        TTCCLayout instances automatically retrieve the nested diagnostic
        context for the current thread without any user intervention.
        hence, even if a servlet is serving multiple clients
        simultaneously, the logs emanating from the same code (belonging to
        the same logger) can still be distinguished because each client
        request will have a different ndc tag.

        <p>heavy duty systems should call the #remove method when
        leaving the run method of a thread. this ensures that the memory
        used by the thread can be freed by the java garbage
        collector. there is a mechanism to lazily remove references to dead
        threads. in practice, this means that you can be a little sloppy
        and sometimes forget to call #remove before exiting a
        thread.

        */
        class LOG4CXX_EXPORT NDC
        {
        public:
                /**
                 *  Pair of Message and FullMessage.
                 */
                typedef std::pair<LogString, LogString> DiagnosticContext;
                typedef std::stack<DiagnosticContext> Stack;

                /**
                 Creates a nested diagnostic context.
                 Since java performs no automatic cleanup of objects when a
                 scope is left, in log4j push() and pop() must be used
                 to manage the NDC. For convenience, log4cxx provides 
                 an NDC constructor and destructor which simply call the push() and
                 pop() methods, allowing for automatic cleanup when the current
                 scope ends.

                 @param message The new diagnostic context information.
                 @see The #push method.
                 */
                NDC(const std::string& message);

                /**
                Removes the topmost element from the NDC stack.

                @see The #pop method.
                */
                ~NDC();

                /**
                Clear any nested diagnostic information if any. This method is
                useful in cases where the same thread can be potentially used
                over and over in different unrelated contexts.
                */
                static void clear();

                /**
                    Clone the diagnostic context for the current thread.
                    <p>Internally a diagnostic context is represented as a stack.  A
                    given thread can supply the stack (i.e. diagnostic context) to a
                    child thread so that the child can inherit the parent thread's
                    diagnostic context.
                    <p>The child thread uses the #inherit method to
                    inherit the parent's diagnostic context.
                    <p>If not passed to #inherit, returned stack should be deleted by caller.
                    @return Stack A clone of the current thread's diagnostic context, will not be null.
                */
                static Stack * cloneStack();

                /**
                Inherit the diagnostic context of another thread.
                <p>The parent thread can obtain a reference to its diagnostic
                context using the #cloneStack method.  It should
                communicate this information to its child so that it may inherit
                the parent's diagnostic context.
                <p>The parent's diagnostic context is cloned before being
                inherited. In other words, once inherited, the two diagnostic
                contexts can be managed independently.
                @param stack The diagnostic context of the parent thread,
                    will be deleted during call.  If NULL, NDC will not be modified.
                */
                static void inherit(Stack * stack);

                /**
                 *   Get the current value of the NDC of the
                 *   currrent thread.
                * @param dest destination to which to append content of NDC.
                * @return true if NDC is set.
                */
                static bool get(LogString& dest);

                /**
                Get the current nesting depth of this diagnostic context.
                */
                static int getDepth();


                /**
                * Tests if the NDC is empty.
                */
                static bool empty();

                /**
                Pop top value off stack.
                @return top value.
                */
                static LogString pop();
                /**
                Pop top value off stack.
                @param buf to which top value is appended.
                @return true if NDC contained at least one value.
                */
                static bool pop(std::string& buf);

                /**
                Looks at the last diagnostic context at the top of this NDC
                without removing it.
                <p>The returned value is the value that was pushed last. If no
                context is available, then the empty string "" is returned.
                @return String The innermost diagnostic context.
                */
                static LogString peek();
                /**
                Get top value without removing value.
                @param buf to which top value is appended.
                @return true if NDC contained at least one value.
                */
                static bool peek(std::string& buf);

                /**
                Push new diagnostic context information for the current thread.
                <p>The contents of the <code>message</code> parameter is
                determined solely by the client.
                @param message The new diagnostic context information.
                */
                static void push(const std::string& message);
                /**
                Push new diagnostic context information for the current thread.
                <p>The contents of the <code>message</code> parameter is
                determined solely by the client.
                @param message The new diagnostic context information.
                */
                static void pushLS(const LogString& message);

                /**
                Remove the diagnostic context for this thread.
                <p>Each thread that created a diagnostic context by calling
                #push should call this method before exiting. Otherwise,
                the memory used by the <b>thread</b> cannot be reclaimed by the
                VM.
                <p>As this is such an important problem in heavy duty systems and
                because it is difficult to always guarantee that the remove
                method is called before exiting a thread, this method has been
                augmented to lazily remove references to dead threads. In
                practice, this means that you can be a little sloppy and
                occasionally forget to call #remove before exiting a
                thread. However, you must call <code>remove</code> sometime. If
                you never call it, then your application is sure to run out of
                memory.
                */
                static void remove();
                
#if LOG4CXX_WCHAR_T_API
               /**
                 Creates a nested diagnostic context.
                 Since java performs no automatic cleanup of objects when a
                 scope is left, in log4j push() and pop() must be used
                 to manage the NDC. For convenience, log4cxx provides 
                 an NDC constructor and destructor which simply call the push() and
                 pop() methods, allowing for automatic cleanup when the current
                 scope ends.

                 @param message The new diagnostic context information.
                 @see The #push method.
                 */
                 NDC(const std::wstring& message);
                 /**
                Push new diagnostic context information for the current thread.
                <p>The contents of the <code>message</code> parameter is
                determined solely by the client.
                @param message The new diagnostic context information.
                */
               static void push(const std::wstring& message);
                /**
                 *   Appends the current NDC content to the provided string.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool peek(std::wstring& dst);
                /**
                 *   Appends the current NDC content to the provided string and removes the value from the NDC.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool pop(std::wstring& dst);
#endif
#if LOG4CXX_UNICHAR_API
               /**
                 Creates a nested diagnostic context.
                 Since java performs no automatic cleanup of objects when a
                 scope is left, in log4j push() and pop() must be used
                 to manage the NDC. For convenience, log4cxx provides 
                 an NDC constructor and destructor which simply call the push() and
                 pop() methods, allowing for automatic cleanup when the current
                 scope ends.

                 @param message The new diagnostic context information.
                 @see The #push method.
                 */
                 NDC(const std::basic_string<UniChar>& message);
                /**
                Push new diagnostic context information for the current thread.
                <p>The contents of the <code>message</code> parameter is
                determined solely by the client.
                @param message The new diagnostic context information.
                */
                static void push(const std::basic_string<UniChar>& message);
                /**
                 *   Appends the current NDC content to the provided string.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool peek(std::basic_string<UniChar>& dst);
                /**
                 *   Appends the current NDC content to the provided string and removes the value from the NDC.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool pop(std::basic_string<UniChar>& dst);
#endif
#if LOG4CXX_CFSTRING_API
               /**
                 Creates a nested diagnostic context.
                 Since java performs no automatic cleanup of objects when a
                 scope is left, in log4j push() and pop() must be used
                 to manage the NDC. For convenience, log4cxx provides 
                 an NDC constructor and destructor which simply call the push() and
                 pop() methods, allowing for automatic cleanup when the current
                 scope ends.

                 @param message The new diagnostic context information.
                 @see The #push method.
                 */
                 NDC(const CFStringRef& message);
                /**
                Push new diagnostic context information for the current thread.
                <p>The contents of the <code>message</code> parameter is
                determined solely by the client.
                @param message The new diagnostic context information.
                */
                static void push(const CFStringRef& message);
                /**
                 *   Gets the current NDC value.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool peek(CFStringRef& dst);
                /**
                 *  Gets and removes the current NDC value.
                 *   @param dst destination.
                 *   @return true if NDC value set. 
                 */
                static bool pop(CFStringRef& dst);
#endif
                
        private:
                NDC(const NDC&);
                NDC& operator=(const NDC&);
                static LogString& getMessage(DiagnosticContext& ctx);
                static LogString& getFullMessage(DiagnosticContext& ctx);
        }; // class NDC;
}  // namespace log4cxx

#if defined(_MSC_VER)
#pragma warning (pop)
#endif


#endif // _LOG4CXX_NDC_H
