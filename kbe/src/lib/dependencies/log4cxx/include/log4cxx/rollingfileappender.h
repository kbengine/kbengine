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
#ifndef _LOG4CXX_ROLLING_FILE_APPENDER_H
#define _LOG4CXX_ROLLING_FILE_APPENDER_H

#if defined(_MSC_VER)
#pragma warning ( push )
#pragma warning ( disable: 4231 4251 4275 4786 )
#endif

#include <log4cxx/rolling/rollingfileappenderskeleton.h>

namespace log4cxx
{

  /** RollingFileAppender extends FileAppender to backup the log files when they reach a certain size. */
  class LOG4CXX_EXPORT RollingFileAppender : public log4cxx::rolling::RollingFileAppenderSkeleton
  {
  private:
    /** The default maximum file size is 10MB. */
    long maxFileSize;

    /** There is one backup file by default. */
    int maxBackupIndex;

  public:
    //
    //   Use custom class to use non-default name to avoid
    //       conflict with log4cxx::rolling::RollingFileAppender
    DECLARE_LOG4CXX_OBJECT_WITH_CUSTOM_CLASS( RollingFileAppender, ClassRollingFileAppender )
    BEGIN_LOG4CXX_CAST_MAP()
         LOG4CXX_CAST_ENTRY( RollingFileAppender )
         LOG4CXX_CAST_ENTRY_CHAIN( FileAppender )
    END_LOG4CXX_CAST_MAP()
    /** The default constructor simply calls its {@link FileAppender#FileAppender parents constructor}. */
    RollingFileAppender();

    /**
                    Instantiate a RollingFileAppender and open the file designated by
     <code>filename</code>. The opened filename will become the ouput destination for this appender.

    <p>If the <code>append</code> parameter is true, the file will be appended to. Otherwise, the file desginated by
     <code>filename</code> will be truncated before being opened.
    */
    RollingFileAppender( const LayoutPtr & layout, const LogString & fileName, bool append );

    /**
                    Instantiate a FileAppender and open the file designated by
     <code>filename</code>. The opened filename will become the output destination for this appender.
     <p>The file will be appended to.
    */
    RollingFileAppender( const LayoutPtr & layout, const LogString & fileName );

    virtual ~RollingFileAppender();

    /** Returns the value of the <b>MaxBackupIndex</b> option. */
    int getMaxBackupIndex() const;

    /** Get the maximum size that the output file is allowed to reach before being rolled over to backup files. */
    long getMaximumFileSize() const;


    /**
                    Set the maximum number of backup files to keep around.

    <p>The <b>MaxBackupIndex</b> option determines how many backup
     files are kept before the oldest is erased. This option takes
     a positive integer value. If set to zero, then there will be no
     backup files and the log file will be truncated when it reaches <code>MaxFileSize</code>.
    */
    void setMaxBackupIndex( int maxBackupIndex );

    /**
                    Set the maximum size that the output file is allowed to reach before being rolled over to backup files.

    <p>In configuration files, the <b>MaxFileSize</b> option takes an
     long integer in the range 0 - 2^63. You can specify the value with the suffixes "KB", "MB" or "GB" so that the integer is
     interpreted being expressed respectively in kilobytes, megabytes
     or gigabytes. For example, the value "10KB" will be interpreted as 10240.
    */
    void setMaxFileSize( const LogString & value );

    void setMaximumFileSize( int value );


    virtual void setOption( const LogString & option, const LogString & value );

    /** Prepares RollingFileAppender for use. */
    void activateOptions( log4cxx::helpers::Pool & pool );


      }; // class RollingFileAppender
      LOG4CXX_PTR_DEF(RollingFileAppender);

    } // namespace log4cxx


#if defined(_MSC_VER)
#pragma warning ( pop )
#endif

#endif //_LOG4CXX_ROLLING_FILE_APPENDER_H
