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
#include <log4cxx/helpers/simpledateformat.h>

#include <apr_time.h>
#include <apr_strings.h>
#include <sstream>
#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/helpers/stringhelper.h>
#include <assert.h>
#if !defined(LOG4CXX)
#define LOG4CXX 1
#endif
#include <log4cxx/private/log4cxx_private.h>
#include <log4cxx/helpers/pool.h>

using namespace log4cxx;
using namespace log4cxx::helpers;

using namespace std;

#if LOG4CXX_HAS_STD_LOCALE
#include <locale>
#endif

#if defined(_MSC_VER) && _MSC_VER < 1300
#define HAS_FACET(locale, type) _HAS(locale, type)
#define USE_FACET(locale, type) _USE(locale, type)
#define PUT_FACET(facet, os, time, spec) facet.put(os, os, time, spec)
#else
#if defined(_RWSTD_NO_TEMPLATE_ON_RETURN_TYPE)
#define HAS_FACET(locale, type) std::has_facet(locale, (type*) 0)
#define USE_FACET(locale, type) std::use_facet(locale, (type*) 0)
#else
#define HAS_FACET(locale, type) std::has_facet < type >(locale)
#define USE_FACET(locale, type) std::use_facet < type >(locale)
#endif
#define PUT_FACET(facet, os, time, spec) facet.put(os, os, os.fill(), time, spec)
#endif

namespace log4cxx
{
  namespace helpers
  {
    namespace SimpleDateFormatImpl
    {
    typedef void (*incrementFunction)(tm& time, apr_time_exp_t& apr_time);

    /**
     * Abstract inner class representing one format token
     * (one or more instances of a character).
     */
    class PatternToken {
    public:
          PatternToken() {
          }

          virtual ~PatternToken() {
          }

          /**
           * Sets the time zone.
           * @param zone new time zone.
           */
          virtual void setTimeZone(const TimeZonePtr& zone) {
          }

          /**
           * Appends the formatted content to the string.
           * @param s string to which format contribution is appended.
           * @param date exploded date/time.
           * @param p memory pool.
           */
          virtual void format(LogString& s,
                              const apr_time_exp_t& date,
                              log4cxx::helpers::Pool& p) const = 0;
                              
    protected:
           
           static void incrementMonth(tm& time, apr_time_exp_t& aprtime) {
               time.tm_mon++;
               aprtime.tm_mon++;
           }

           static void incrementDay(tm& time, apr_time_exp_t& aprtime) {
               time.tm_wday++;
               aprtime.tm_wday++;
           }

           static void incrementHalfDay(tm& time, apr_time_exp_t& aprtime) {
               time.tm_hour += 12;
               aprtime.tm_hour += 12;
           }
           
           static void renderFacet(const std::locale* locale, 
                incrementFunction inc, 
                char spec, 
                unsigned int wspec, 
                const char* aprspec, 
                std::vector<LogString>& values) {
                std::vector<LogString>::iterator valueIter = values.begin();
                tm time;
                memset(&time, 0, sizeof(time));
                apr_time_exp_t aprtime;
                memset(&aprtime, 0, sizeof(aprtime));
#if LOG4CXX_HAS_STD_LOCALE
                if (locale != NULL) {
#if LOG4CXX_WCHAR_T_API
                    if (HAS_FACET(*locale, std::time_put<wchar_t>)) {
                        const std::time_put<wchar_t>& facet = USE_FACET(*locale, std::time_put<wchar_t>);
                        size_t start = 0;
                        std::basic_ostringstream<wchar_t> os;
                        for(; valueIter != values.end(); valueIter++) {
                            PUT_FACET(facet, os, &time, (wchar_t) wspec);
                            Transcoder::decode(os.str().substr(start), *valueIter);
                            start = os.str().length();
                            (*inc)(time, aprtime);
                        }
                    } else 
#endif
                    if (HAS_FACET(*locale,  std::time_put<char>)) {
                        const std::time_put<char>& facet = USE_FACET(*locale, std::time_put<char> );
                        size_t start = 0;
                        std::ostringstream os;
                        for(; valueIter != values.end(); valueIter++) {
                            PUT_FACET(facet, os, &time, spec);
                            Transcoder::decode(os.str().substr(start), *valueIter);
                            start = os.str().length();
                            (*inc)(time, aprtime);
                        }
                    }
                }
#endif          
                const size_t BUFSIZE = 256; 
                char buf[BUFSIZE];
                memset(buf, 0, BUFSIZE);
                apr_size_t retsize = 0;
                for(; valueIter != values.end(); valueIter++) {
                    apr_status_t stat = apr_strftime(buf, &retsize, BUFSIZE, aprspec, &aprtime);
                    (*inc)(time, aprtime);
                    if (stat == APR_SUCCESS) {
                        Transcoder::decode(std::string(buf, retsize), *valueIter);
                    } else {
                        valueIter->append(1, (logchar) 0x3F);
                    }
                }
            }
                              
    private:
          /**
           * Private copy constructor.
           */
          PatternToken(const PatternToken&);

          /**
           * Private assignment operator.
           */
          PatternToken& operator=(const PatternToken&);
    };


class LiteralToken : public PatternToken
{
public:
  LiteralToken( logchar ch1, int count1 ) : ch( ch1 ), count( count1 )
  {
  }

  void format( LogString& s, const apr_time_exp_t & , Pool & /* p */ ) const
  {
    s.append( count, ch );
  }

private:
  logchar ch;
  int count;
};



class EraToken : public PatternToken
{
public:
  EraToken( int /* count */ , const std::locale * /* locale */  )
  {
  }

  void format(LogString& s, const apr_time_exp_t & /* tm */, Pool & /* p */ ) const
  {
    s.append(1, (logchar) 0x41 /* 'A' */);
    s.append(1, (logchar) 0x44 /* 'D' */);
  }
};



class NumericToken : public PatternToken
{
public:
  NumericToken( size_t width1 ) : width( width1 )
  {
  }

  virtual int getField( const apr_time_exp_t & tm ) const = 0;

  void format( LogString& s, const apr_time_exp_t & tm, Pool & p ) const
  {
    size_t initialLength = s.length();
    
    StringHelper::toString( getField( tm ), p, s );
    size_t finalLength = s.length();
    if ( initialLength + width > finalLength )
    {
      s.insert( initialLength, ( initialLength + width ) - finalLength, (logchar) 0x30 /* '0' */);
    }
  }

private:
  size_t width;
  char zeroDigit;
};



class YearToken : public NumericToken
{
public:
  YearToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return 1900 + tm.tm_year;
  }
};



class MonthToken : public NumericToken
{
public:
  MonthToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_mon + 1;
  }
};



class AbbreviatedMonthNameToken : public PatternToken
{
public:
  AbbreviatedMonthNameToken(int, const std::locale *locale) : names( 12 ) {
      renderFacet(locale, PatternToken::incrementMonth, 'b', 0x62, "%b", names);
  }

  void format(LogString& s, const apr_time_exp_t & tm, Pool & /* p */ ) const
  {
    s.append( names[tm.tm_mon] );
  }

private:
  std::vector < LogString > names;
};



class FullMonthNameToken : public PatternToken
{
public:
  FullMonthNameToken( int width, const std::locale *locale) : names( 12 )
  {
      renderFacet(locale, PatternToken::incrementMonth, 'B', 0x42, "%B", names);
  }

  void format( LogString& s, const apr_time_exp_t & tm, Pool & /* p */ ) const
  {
    s.append( names[tm.tm_mon] );
  }

private:
  std::vector < LogString > names;
};



class WeekInYearToken : public NumericToken
{
public:
  WeekInYearToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_yday / 7;
  }
};



class WeekInMonthToken : public NumericToken
{
public:
  WeekInMonthToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_mday / 7;
  }
};



class DayInMonthToken : public NumericToken
{
public:
  DayInMonthToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_mday;
  }
};



class DayInYearToken : public NumericToken
{
public:
  DayInYearToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_yday;
  }
};



class DayOfWeekInMonthToken : public NumericToken
{
public:
  DayOfWeekInMonthToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & /* tm */ ) const
  {
    return -1;
  }
};



class AbbreviatedDayNameToken : public PatternToken
{
public:
  AbbreviatedDayNameToken( int width, const std::locale *locale) : names( 7 ) {
      renderFacet(locale, PatternToken::incrementDay, 'a', 0x61, "%a", names);
  }

  void format( LogString& s, const apr_time_exp_t & tm, Pool & /* p */ ) const
  {
    s.append( names[tm.tm_wday] );
  }

private:
  std::vector < LogString > names;

};



class FullDayNameToken : public PatternToken
{
public:
  FullDayNameToken( int width, const std::locale *locale) : names( 7 ) {
      renderFacet(locale, PatternToken::incrementDay, 'A', 0x41, "%A", names);
  }

  void format( LogString& s, const apr_time_exp_t & tm, Pool & /* p */ ) const
  {
    s.append( names[tm.tm_wday] );
  }

private:
  std::vector < LogString > names;

};



class MilitaryHourToken : public NumericToken
{
public:
  MilitaryHourToken( int width1, int offset1 ) : NumericToken( width1 ), offset( offset1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_hour + offset;
  }

private:
  int offset;
};



class HourToken : public NumericToken
{
public:
  HourToken( int width1, int /* offset1 */ ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return ( ( tm.tm_hour + 12 - offset ) % 12 ) + offset;
  }

private:
  int offset;
};



class MinuteToken : public NumericToken
{
public:
  MinuteToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_min;
  }
};



class SecondToken : public NumericToken
{
public:
  SecondToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_sec;
  }
};



class MillisecondToken : public NumericToken
{
public:
  MillisecondToken( int width1 ) : NumericToken( width1 )
  {
  }

  int getField( const apr_time_exp_t & tm ) const
  {
    return tm.tm_usec / 1000;
  }
};



class AMPMToken : public PatternToken
{
public:
  AMPMToken( int width, const std::locale *locale) : names( 2 ) {
      renderFacet(locale, PatternToken::incrementHalfDay, 'p', 0x70, "%p", names);
  }

  void format( LogString& s, const apr_time_exp_t & tm, Pool & /* p */ ) const
  {
    s.append( names[tm.tm_hour / 12] );
  }

private:
  std::vector < LogString > names;
};



class GeneralTimeZoneToken : public PatternToken
{
public:
  GeneralTimeZoneToken( int /* width */ )
  {
  }

  void format( LogString& s, const apr_time_exp_t & , Pool & /* p */ ) const {
    s.append(timeZone->getID());
  }

  void setTimeZone( const TimeZonePtr & zone )
  {
    timeZone = zone;
  }

private:
  TimeZonePtr timeZone;
};



class RFC822TimeZoneToken : public PatternToken
{
public:
  RFC822TimeZoneToken( int /* width */ )
  {
  }

  void format( LogString& s, const apr_time_exp_t & tm, Pool & p ) const
  {
    if ( tm.tm_gmtoff == 0 )
    {
      s.append( 1, (logchar) 0x5A /* 'Z'  */ );
    }
    else
    {
      apr_int32_t off = tm.tm_gmtoff;
      size_t basePos = s.length();
      s.append( LOG4CXX_STR( "+0000" ) );
      if ( off < 0 )
      {
        s[basePos] = 0x2D; // '-'
        off = -off;
      }
      LogString hours;
      StringHelper::toString( off / 3600, p, hours );
      size_t hourPos = basePos + 2;
      //
      //   assumes that point values for 0-9 are same between char and wchar_t
      //
      for ( size_t i = hours.length(); i-- > 0; )
      {
        s[hourPos--] = hours[i];
      }
      LogString min;
      StringHelper::toString( ( off % 3600 ) / 60, p, min );
      size_t minPos = basePos + 4;
      //
      //   assumes that point values for 0-9 are same between char and wchar_t
      //
      for ( size_t j = min.length(); j-- > 0; )
      {
        s[minPos--] = min[j];
      }
    }
  }
};




  }
}
}


using namespace log4cxx::helpers::SimpleDateFormatImpl;

void SimpleDateFormat::addToken(const logchar spec, const int repeat, const std::locale * locale,
           std::vector < PatternToken * > & pattern )
           {
             PatternToken * token = NULL;
             switch ( spec )
             {
               case 0x47: // 'G' 
                 token = ( new EraToken( repeat, locale ) );
               break;

               case 0x79: // 'y'
                 token = ( new YearToken( repeat ) );
               break;

               case 0x4D: // 'M'
                 if ( repeat <= 2 )
                 {
                   token = ( new MonthToken( repeat ) );
                 }
                 else if ( repeat <= 3 )
                 {
                   token = ( new AbbreviatedMonthNameToken( repeat, locale ) );
                 }
                 else
                 {
                   token = ( new FullMonthNameToken( repeat, locale ) );
                 }
               break;

               case 0x77: // 'w'
                 token = ( new WeekInYearToken( repeat ) );
               break;

               case 0x57: // 'W'
                 token = ( new WeekInMonthToken( repeat ) );
               break;

               case 0x44: // 'D'
                 token = ( new DayInYearToken( repeat ) );
               break;

               case 0x64: // 'd'
                 token = ( new DayInMonthToken( repeat ) );
               break;

               case 0x46: // 'F'
                 token = ( new DayOfWeekInMonthToken( repeat ) );
               break;

               case 0x45: // 'E'
                 if ( repeat <= 3 )
                 {
                   token = ( new AbbreviatedDayNameToken( repeat, locale ) );
                 }
                 else
                 {
                   token = ( new FullDayNameToken( repeat, locale ) );
                 }
               break;

               case 0x61: // 'a'
                 token = ( new AMPMToken( repeat, locale ) );
               break;

               case 0x48: // 'H'
                 token = ( new MilitaryHourToken( repeat, 0 ) );
               break;

               case 0x6B: // 'k'
                 token = ( new MilitaryHourToken( repeat, 1 ) );
               break;

               case 0x4B: // 'K'
                 token = ( new HourToken( repeat, 0 ) );
               break;

               case 0x68: // 'h'
                 token = ( new HourToken( repeat, 1 ) );
               break;

               case 0x6D: // 'm' 
                 token = ( new MinuteToken( repeat ) );
               break;

               case 0x73: // 's' 
                 token = ( new SecondToken( repeat ) );
               break;

               case 0x53: // 'S'
                 token = ( new MillisecondToken( repeat ) );
               break;

               case 0x7A: // 'z'
                 token = ( new GeneralTimeZoneToken( repeat ) );
               break;

               case 0x5A: // 'Z'
                 token = ( new RFC822TimeZoneToken( repeat ) );
               break;

               default:
                 token = ( new LiteralToken( spec, repeat ) );
             }
             assert( token != NULL );
             pattern.push_back( token );
}


void SimpleDateFormat::parsePattern( const LogString & fmt, const std::locale * locale,
           std::vector < PatternToken * > & pattern )
{
             if ( !fmt.empty() )
             {
               LogString::const_iterator iter = fmt.begin();
               int repeat = 1;
               logchar prevChar = * iter;
               for ( iter++; iter != fmt.end(); iter++ )
               {
                 if ( * iter == prevChar )
                 {
                   repeat++;
                 }
                 else
                 {
                   addToken( prevChar, repeat, locale, pattern );
                   prevChar = * iter;
                   repeat = 1;
                 }
               }
               addToken( prevChar, repeat, locale, pattern );
             }
}


SimpleDateFormat::SimpleDateFormat( const LogString & fmt ) : timeZone( TimeZone::getDefault() )
{
#if LOG4CXX_HAS_STD_LOCALE
  std::locale defaultLocale;
  parsePattern( fmt, & defaultLocale, pattern );
#else
  parsePattern( fmt, NULL, pattern );
#endif
  for ( PatternTokenList::iterator iter = pattern.begin(); iter != pattern.end(); iter++ )
  {
    ( * iter )->setTimeZone( timeZone );
  }
}

SimpleDateFormat::SimpleDateFormat( const LogString & fmt, const std::locale * locale ) : timeZone( TimeZone::getDefault() )
{
  parsePattern( fmt, locale, pattern );
  for ( PatternTokenList::iterator iter = pattern.begin(); iter != pattern.end(); iter++ )
  {
    ( * iter )->setTimeZone( timeZone );
  }
}


SimpleDateFormat::~SimpleDateFormat()
{
  for ( PatternTokenList::iterator iter = pattern.begin(); iter != pattern.end(); iter++ )
  {
    delete * iter;
  }
}


void SimpleDateFormat::format( LogString & s, log4cxx_time_t time, Pool & p ) const
{
  apr_time_exp_t exploded;
  apr_status_t stat = timeZone->explode( & exploded, time );
  if ( stat == APR_SUCCESS )
  {
    for ( PatternTokenList::const_iterator iter = pattern.begin(); iter != pattern.end(); iter++ )
    {
      ( * iter )->format( s, exploded, p );
    }
  }
}

void SimpleDateFormat::setTimeZone( const TimeZonePtr & zone )
{
  timeZone = zone;
}
