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
#ifndef _LOG4CXX_FILTER_LOCATIONINFOFILTER_H
#define _LOG4CXX_FILTER_LOCATIONINFOFILTER_H

#include <log4cxx/spi/filter.h>

namespace log4cxx
{
    namespace rule
    {
        class ExpressionRule;
        class Rule;
        typedef helpers::ObjectPtrT < Rule > RulePtr;
        typedef helpers::ObjectPtrT < ExpressionRule > ExpressionRulePtr;
    }

    namespace filter
    {
/**
 * Location information is usually specified at the appender level - all events associated
 * with an appender either create and parse stack traces or they do not.  This is
 * an expensive operation and in some cases not needed for all events associated with
 * an appender.
 *
 * This filter creates event-level location information only if the provided expression evaluates to true.
 *
 * For information on expression syntax, see org.apache.log4j.rule.ExpressionRule
 *
 * 
 */
        class LOG4CXX_EXPORT LocationInfoFilter:public log4cxx::spi::Filter
        {
            bool convertInFixToPostFix;
            LogString expression;
                      log4cxx::rule::RulePtr expressionRule;
            //HACK: Category is the last of the internal layers - pass this in as the class name
            //in order for parsing to work correctly
            LogString className;

          public:
                      DECLARE_LOG4CXX_OBJECT(LocationInfoFilter)
                      BEGIN_LOG4CXX_CAST_MAP()
                      LOG4CXX_CAST_ENTRY(log4cxx::spi::Filter)
                      END_LOG4CXX_CAST_MAP()

                      LocationInfoFilter();

            void activateOptions(log4cxx::helpers::Pool &);

            void setExpression(const LogString & expression);

            LogString getExpression() const;

            void setConvertInFixToPostFix(bool convertInFixToPostFix);

            bool getConvertInFixToPostFix() const;

  /**
   * If this event does not already contain location information,
   * evaluate the event against the expression.
   *
   * If the expression evaluates to true, generate a LocationInfo instance
   * by creating an exception and set this LocationInfo on the event.
   *
   * Returns {@link log4cxx::spi::Filter#NEUTRAL}
   */
            FilterDecision decide(const spi::LoggingEventPtr & event) const;

        };
    }
}
#endif
