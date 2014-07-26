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

#ifndef _LOG4CXX_NET_SMTP_H
#define _LOG4CXX_NET_SMTP_H


#include <log4cxx/appenderskeleton.h>
#include <log4cxx/helpers/cyclicbuffer.h>
#include <log4cxx/spi/triggeringeventevaluator.h>

namespace log4cxx
{
        namespace net
        {
                /**
                Send an e-mail when a specific logging event occurs, typically on
                errors or fatal errors.
                <p>The number of logging events delivered in this e-mail depend on
                the value of <b>BufferSize</b> option. The
                <code>SMTPAppender</code> keeps only the last
                <code>BufferSize</code> logging events in its cyclic buffer. This
                keeps memory requirements at a reasonable level while still
                delivering useful application context.
                */
                class LOG4CXX_EXPORT SMTPAppender : public AppenderSkeleton
                {
                private:

                private:
                        SMTPAppender(const SMTPAppender&);
                        SMTPAppender& operator=(const SMTPAppender&);
                        static bool asciiCheck(const LogString& value, const LogString& label);
                        /**
                        This method determines if there is a sense in attempting to append.
                        <p>It checks whether there is a set output target and also if
                        there is a set layout. If these checks fail, then the boolean
                        value <code>false</code> is returned. */
                        bool checkEntryConditions();

                        LogString to;
                        LogString cc;
                        LogString bcc;
                        LogString from;
                        LogString subject;
                        LogString smtpHost;
                        LogString smtpUsername;
                        LogString smtpPassword;
                        int smtpPort;
                        int bufferSize; // 512
                        bool locationInfo;
                        helpers::CyclicBuffer cb;
                        spi::TriggeringEventEvaluatorPtr evaluator;

                public:
                        DECLARE_LOG4CXX_OBJECT(SMTPAppender)
                        BEGIN_LOG4CXX_CAST_MAP()
                                LOG4CXX_CAST_ENTRY(SMTPAppender)
                                LOG4CXX_CAST_ENTRY_CHAIN(AppenderSkeleton)
                        END_LOG4CXX_CAST_MAP()

                        SMTPAppender();
                        /**
                        The default constructor will instantiate the appender with a
                        spi::TriggeringEventEvaluator that will trigger on events with
                        level ERROR or higher.*/
                        SMTPAppender(log4cxx::helpers::Pool& p);

                        /**
                        Use <code>evaluator</code> passed as parameter as the
                        spi::TriggeringEventEvaluator for this net::SMTPAppender.
                        */
                        SMTPAppender(spi::TriggeringEventEvaluatorPtr evaluator);

                        ~SMTPAppender();

                        /**
                         Set options
                        */
                        virtual void setOption(const LogString& option, const LogString& value);

                        /**
                        Activate the specified options, such as the smtp host, the
                        recipient, from, etc.
                        */
                        virtual void activateOptions(log4cxx::helpers::Pool& p);

                        /**
                        Perform SMTPAppender specific appending actions, mainly adding
                        the event to a cyclic buffer and checking if the event triggers
                        an e-mail to be sent. */
                        virtual void append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p);


                        virtual void close();

                        /**
                        Returns value of the <b>To</b> option.
                        */
                        LogString getTo() const;

                        /**
                        Returns value of the <b>cc</b> option.
                        */
                        LogString getCc() const;

                        /**
                        Returns value of the <b>bcc</b> option.
                        */
                        LogString getBcc() const;


                        /**
                        The <code>SMTPAppender</code> requires a {@link
                        Layout layout}.  */
                        virtual bool requiresLayout() const;

                        /**
                        Send the contents of the cyclic buffer as an e-mail message.
                        */
                        void sendBuffer(log4cxx::helpers::Pool& p);


                        /**
                        Returns value of the <b>EvaluatorClass</b> option.
                        */
                        LogString getEvaluatorClass();

                        /**
                        Returns value of the <b>From</b> option.
                        */
                        LogString getFrom() const;

                        /**
                        Returns value of the <b>Subject</b> option.
                        */
                        LogString getSubject() const;


                        /**
                        The <b>From</b> option takes a string value which should be a
                        e-mail address of the sender.
                        */
                        void setFrom(const LogString& from);

                        /**
                        The <b>Subject</b> option takes a string value which should be a
                        the subject of the e-mail message.
                        */
                        void setSubject(const LogString& subject);

                        /**
                        The <b>BufferSize</b> option takes a positive integer
                        representing the maximum number of logging events to collect in a
                        cyclic buffer. When the <code>BufferSize</code> is reached,
                        oldest events are deleted as new events are added to the
                        buffer. By default the size of the cyclic buffer is 512 events.
                        */
                        void setBufferSize(int bufferSize);

                        /**
                        The <b>SMTPHost</b> option takes a string value which should be a
                        the host name of the SMTP server that will send the e-mail message.
                        */
                        void setSMTPHost(const LogString& smtpHost);

                        /**
                        Returns value of the <b>SMTPHost</b> option.
                        */
                        LogString getSMTPHost() const;

                        /**
                        The <b>SMTPPort</b> option takes a string value which should be a
                        the port of the SMTP server that will send the e-mail message.
                        */
                        void setSMTPPort(int port);

                        /**
                        Returns value of the <b>SMTPHost</b> option.
                        */
                        int getSMTPPort() const;

                        /**
                        The <b>To</b> option takes a string value which should be a
                        comma separated list of e-mail address of the recipients.
                        */
                        void setTo(const LogString& to);

                        /**
                        The <b>Cc</b> option takes a string value which should be a
                        comma separated list of e-mail address of the cc'd recipients.
                        */
                        void setCc(const LogString& to);

                        /**
                        The <b>Bcc</b> option takes a string value which should be a
                        comma separated list of e-mail address of the bcc'd recipients.
                        */
                        void setBcc(const LogString& to);


                        /**
                        The <b>SMTPUsername</b> option takes a string value which should be a
                        the user name for the SMTP server.
                        */
                        void setSMTPUsername(const LogString& newVal);

                        /**
                        Returns value of the <b>SMTPUsername</b> option.
                        */
                        LogString getSMTPUsername() const;

                        /**
                        The <b>SMTPPassword</b> option takes a string value which should be a
                        the password for the SMTP server.
                        */
                        void setSMTPPassword(const LogString& newVal);

                        /**
                        Returns value of the <b>SMTPPassword</b> option.
                        */
                        LogString getSMTPPassword() const;

                        /**
                        Returns value of the <b>BufferSize</b> option.
                        */
                        inline int getBufferSize() const
                                { return bufferSize; }

                   
                        /**
                         *   Gets the current triggering evaluator.
                         *   @return triggering evaluator.
                         */     
                        log4cxx::spi::TriggeringEventEvaluatorPtr getEvaluator() const;

                        /**
                         *   Sets the triggering evaluator.
                         *   @param trigger triggering evaluator.
                         */     
                        void setEvaluator(log4cxx::spi::TriggeringEventEvaluatorPtr& trigger);

                        /**
                        The <b>EvaluatorClass</b> option takes a string value
                        representing the name of the class implementing the
                        spi::TriggeringEventEvaluator interface. A corresponding object will
                        be instantiated and assigned as the triggering event evaluator
                        for the SMTPAppender.
                        */
                        void setEvaluatorClass(const LogString& value);
                 
                        /**
                        The <b>LocationInfo</b> option is provided for compatibility with log4j
                        and has no effect in log4cxx.
                        */
                        void setLocationInfo(bool locationInfo);

                        /**
                        Returns value of the <b>LocationInfo</b> option.
                        */
                        bool getLocationInfo() const;
                }; // class SMTPAppender
                
                LOG4CXX_PTR_DEF(SMTPAppender);                

        }  // namespace net
} // namespace log4cxx

#endif // _LOG4CXX_NET_SMTP_H
