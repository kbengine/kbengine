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
#include <log4cxx/db/odbcappender.h>
#include <log4cxx/helpers/loglog.h>
#include <log4cxx/helpers/optionconverter.h>
#include <log4cxx/helpers/stringhelper.h>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/patternlayout.h>
#include <apr_strings.h>

#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#if LOG4CXX_HAVE_ODBC
#if defined(WIN32) || defined(_WIN32)
#include <windows.h>
#endif
#include <sqlext.h>
#endif


using namespace log4cxx;
using namespace log4cxx::helpers;
using namespace log4cxx::db;
using namespace log4cxx::spi;

SQLException::SQLException(short fHandleType, 
                           void* hInput, const char* prolog,
                           log4cxx::helpers::Pool& p) 
                           : Exception(formatMessage(fHandleType, hInput, prolog, p)) {
}


SQLException::SQLException(const char* msg) 
   : Exception(msg) {
}

SQLException::SQLException(const SQLException& src) 
   : Exception(src) {
}

const char* SQLException::formatMessage(short fHandleType,
                          void* hInput, const char* prolog, log4cxx::helpers::Pool& p) {
   std::string strReturn(prolog);
   strReturn.append(" - ");
#if LOG4CXX_HAVE_ODBC
   SQLCHAR       SqlState[6];
   SQLCHAR       Msg[SQL_MAX_MESSAGE_LENGTH];
   SQLINTEGER    NativeError;
   SQLSMALLINT   i;
   SQLSMALLINT   MsgLen;
   SQLRETURN     rc2;

   // Get the status records.
   i = 1;
   while ((rc2 = SQLGetDiagRecA(fHandleType, hInput, i, SqlState, &NativeError,
                        Msg, sizeof(Msg), &MsgLen)) != SQL_NO_DATA) 
   {
      strReturn.append((char*) Msg);
      i++;
   }
#else
   strReturn.append("log4cxx built without ODBC support");
#endif

   return apr_pstrdup((apr_pool_t*) p.getAPRPool(), strReturn.c_str());
}


IMPLEMENT_LOG4CXX_OBJECT(ODBCAppender)



ODBCAppender::ODBCAppender()
: connection(0), env(0), bufferSize(1)
{
}

ODBCAppender::~ODBCAppender()
{
   finalize();
}

void ODBCAppender::setOption(const LogString& option, const LogString& value)
{
   if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("BUFFERSIZE"), LOG4CXX_STR("buffersize")))
   {
      setBufferSize((size_t)OptionConverter::toInt(value, 1));
   }
   else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("PASSWORD"), LOG4CXX_STR("password")))
   {
      setPassword(value);
   }
   else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("SQL"), LOG4CXX_STR("sql")))
   {
      setSql(value);
   }
   else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("URL"), LOG4CXX_STR("url"))
      || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("DSN"), LOG4CXX_STR("dsn"))
      || StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("CONNECTIONSTRING"), LOG4CXX_STR("connectionstring"))  )
   {
      setURL(value);
   }
   else if (StringHelper::equalsIgnoreCase(option, LOG4CXX_STR("USER"), LOG4CXX_STR("user")))
   {
      setUser(value);
   }
   else
   {
      AppenderSkeleton::setOption(option, value);
   }
}

void ODBCAppender::activateOptions(log4cxx::helpers::Pool&) {
#if !LOG4CXX_HAVE_ODBC
    LogLog::error(LOG4CXX_STR("Can not activate ODBCAppender unless compiled with ODBC support."));
#endif
}


void ODBCAppender::append(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p)
{
#if LOG4CXX_HAVE_ODBC
   buffer.push_back(event);

   if (buffer.size() >= bufferSize)
      flushBuffer(p);
#endif      
}

LogString ODBCAppender::getLogStatement(const spi::LoggingEventPtr& event, log4cxx::helpers::Pool& p) const
{
   LogString sbuf;
   getLayout()->format(sbuf, event, p);
   return sbuf;
}

void ODBCAppender::execute(const LogString& sql, log4cxx::helpers::Pool& p)
{
#if LOG4CXX_HAVE_ODBC
   SQLRETURN ret;
   SQLHDBC con = SQL_NULL_HDBC;
   SQLHSTMT stmt = SQL_NULL_HSTMT;

   try
   {
      con = getConnection(p);

      ret = SQLAllocHandle( SQL_HANDLE_STMT, con, &stmt);
      if (ret < 0)
      {
         throw SQLException( SQL_HANDLE_DBC, con, "Failed to allocate sql handle.", p);
      }

      SQLWCHAR* wsql = Transcoder::wencode(sql, p); 
      ret = SQLExecDirectW(stmt, wsql, SQL_NTS);

     if (ret < 0)
      {
         throw SQLException(SQL_HANDLE_STMT, stmt, "Failed to execute sql statement.", p);
      }
   }
   catch (SQLException& e)
   {
      if (stmt != SQL_NULL_HSTMT)
      {
         SQLFreeHandle(SQL_HANDLE_STMT, stmt);
      }
   
      throw e;
   }
   SQLFreeHandle(SQL_HANDLE_STMT, stmt);
   closeConnection(con);
#else
    throw SQLException("log4cxx build without ODBC support");
#endif
}

/* The default behavior holds a single connection open until the appender
is closed (typically when garbage collected).*/
void ODBCAppender::closeConnection(ODBCAppender::SQLHDBC /* con */)
{
}





ODBCAppender::SQLHDBC ODBCAppender::getConnection(log4cxx::helpers::Pool& p)
{
#if LOG4CXX_HAVE_ODBC
   SQLRETURN ret;

   if (env == SQL_NULL_HENV)
   {
      ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &env);
      if (ret < 0)
      {
         SQLException ex(SQL_HANDLE_ENV, env, "Failed to allocate SQL handle.", p);
         env = SQL_NULL_HENV;
         throw ex;
      }

      ret = SQLSetEnvAttr(env, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_INTEGER);
      if (ret < 0)
      {
         SQLException ex(SQL_HANDLE_ENV, env, "Failed to set odbc version.", p);
         SQLFreeHandle(SQL_HANDLE_ENV, env);
         env = SQL_NULL_HENV;
         throw ex;
      }
   }

   if (connection == SQL_NULL_HDBC)
   {
      ret = SQLAllocHandle(SQL_HANDLE_DBC, env, &connection);
      if (ret < 0)
      {
         SQLException ex(SQL_HANDLE_DBC, connection, "Failed to allocate sql handle.", p);
         connection = SQL_NULL_HDBC;
         throw ex;
      }


     SQLWCHAR* wURL = Transcoder::wencode(databaseURL, p);

     wchar_t szOutConnectionString[1024];
     SQLSMALLINT nOutConnctionLength = 0;

     ret = SQLDriverConnectW( connection, NULL, 
            wURL, SQL_NTS, 
            szOutConnectionString, sizeof( szOutConnectionString ),
            &nOutConnctionLength, SQL_DRIVER_NOPROMPT );


     if (ret < 0)
      {
         SQLException ex(SQL_HANDLE_DBC, connection, "Failed to connect to database.", p);
         SQLFreeHandle(SQL_HANDLE_DBC, connection);
         connection = SQL_NULL_HDBC;
         throw ex;
      }
   }

   return connection;
#else
   return 0;
#endif   
}

void ODBCAppender::close()
{
   if (closed) {
       return;
   }
   Pool p;
   try
   {
      flushBuffer(p);
   }
   catch (SQLException& e)
   {
      errorHandler->error(LOG4CXX_STR("Error closing connection"),
         e, ErrorCode::GENERIC_FAILURE);
   }
#if LOG4CXX_HAVE_ODBC
   if (connection != SQL_NULL_HDBC)
   {
      SQLDisconnect(connection);
      SQLFreeHandle(SQL_HANDLE_DBC, connection);
   }

   if (env != SQL_NULL_HENV)
   {
      SQLFreeHandle(SQL_HANDLE_ENV, env);
   }
#endif
   this->closed = true;
}

void ODBCAppender::flushBuffer(Pool& p)
{
   std::list<spi::LoggingEventPtr>::iterator i;
   for (i = buffer.begin(); i != buffer.end(); i++)
   {
      try
      {
         const LoggingEventPtr& logEvent = *i;
         LogString sql = getLogStatement(logEvent, p);
         execute(sql, p);
      }
      catch (SQLException& e)
      {
         errorHandler->error(LOG4CXX_STR("Failed to excute sql"), e,
            ErrorCode::FLUSH_FAILURE);
      }
   }

   // clear the buffer of reported events
   buffer.clear();
}

void ODBCAppender::setSql(const LogString& s)
{
   sqlStatement = s;
   if (getLayout() == 0)
   {
      this->setLayout(new PatternLayout(s));
   }
   else
   {
      PatternLayoutPtr patternLayout = this->getLayout();
      if (patternLayout != 0)
      {
         patternLayout->setConversionPattern(s);
      }
   }
}
