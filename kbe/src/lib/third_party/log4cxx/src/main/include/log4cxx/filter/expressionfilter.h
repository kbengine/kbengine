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

#ifndef _LOG4CXX_FILTER_EXPRESSIONFILTER_H
#define _LOG4CXX_FILTER_EXPRESSIONFILTER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif


#include <log4cxx/spi/filter.h>

namespace log4cxx
{
    namespace rule
    {
        class Rule;
        typedef helpers::ObjectPtrT < Rule > RulePtr;
    }


    namespace filter
    {


/**
 *A filter supporting complex expressions - supports both infix and postfix
 * expressions (infix expressions must first be converted to postfix prior
 * to processing).
 *
 * <p>See <code>org.apache.log4j.chainsaw.LoggingEventFieldResolver.java</code>
 * for the correct names for logging event fields used when building expressions.
 *
 * <p>See <code>org.apache.log4j.chainsaw.rule</code> package for a list of available
 * rules which can be applied using the expression syntax.
 *
 * <p>See <code>org.apache.log4j.chainsaw.RuleFactory</code> for the symbols
 * used to activate the corresponding rules.
 *
 *NOTE:  Grouping using parentheses is supported - all tokens must be separated by spaces, and
 *operands which contain spaces are not yet supported.
 *
 *Example:
 *
 *In order to build a filter that displays all messages with infomsg-45 or infomsg-44 in the message,
 *as well as all messages with a level of WARN or higher, build an expression using
 *the LikeRule (supports ORO-based regular expressions) and the InequalityRule.
 * <b> ( MSG LIKE infomsg-4[4,5] ) && ( LEVEL >= WARN ) </b>
 *
 *Three options are required:
 *  <b>Expression</b> - the expression to match
 *  <b>ConvertInFixToPostFix</b> - convert from infix to posfix (default true)
 *  <b>AcceptOnMatch</b> - true or false (default true)
 *
 * Meaning of <b>AcceptToMatch</b>:
 * If there is a match between the value of the
 * Expression option and the {@link log4cxx::spi::LoggingEvent} and AcceptOnMatch is true,
 * the {@link #decide} method returns {@link log4cxx::spi::Filter#ACCEPT}.
 *
 * If there is a match between the value of the
 * Expression option and the {@link log4cxx::spi::LoggingEvent} and AcceptOnMatch is false,
 * {@link log4cxx::spi::Filter#DENY} is returned.
 *
 * If there is no match, {@link log4cxx::spi::Filter#NEUTRAL} is returned.
 *
 * 
 */
        class LOG4CXX_EXPORT ExpressionFilter:public log4cxx::spi::Filter
        {
          private:
            bool acceptOnMatch;
            bool convertInFixToPostFix;
            LogString expression;
                      log4cxx::rule::RulePtr expressionRule;
                      ExpressionFilter(const ExpressionFilter &);
                  ExpressionFilter & operator=(const ExpressionFilter &);

          public:
                  DECLARE_LOG4CXX_OBJECT(ExpressionFilter)
                  BEGIN_LOG4CXX_CAST_MAP()
                  LOG4CXX_CAST_ENTRY(log4cxx::spi::Filter)
                  END_LOG4CXX_CAST_MAP()


                  ExpressionFilter();

            void activateOptions(log4cxx::helpers::Pool & p);

            void setExpression(const LogString & expression);

            LogString getExpression() const;

            void setConvertInFixToPostFix(bool convertInFixToPostFix);

            bool getConvertInFixToPostFix() const;

            void setAcceptOnMatch(bool acceptOnMatch);

            bool getAcceptOnMatch() const;

  /**
     Returns {@link log4cxx::spi::Filter#NEUTRAL} is there is no string match.
   */
            FilterDecision decide(const spi::LoggingEventPtr & event) const;
        };
    }
}

#if defined(_MSC_VER)
#pragma warning ( pop )
#endif


#endif
