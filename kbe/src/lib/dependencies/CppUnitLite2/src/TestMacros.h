#ifndef TEST_MACROS_H_
#define TEST_MACROS_H_

#ifdef _MSC_VER
    #pragma warning (push)
    #pragma warning (disable : 4267 4389 4018)
#endif

#include "TestResult.h"
#include <sstream>


namespace cppunitlite
{

template < typename Type1, typename Type2 >
void CheckEqual( TestResult& result_, const Type1& expected, const Type2& actual, const char* file, unsigned int line, const char* name)
{                                                                                                   
    try {                                                                                              
        if (!(expected == actual)) {                                                               
            std::stringstream msg;                                                                     
            msg << "expected: '" << (expected) << "' but was: '" << (actual) << "'" << std::ends;   
            result_.AddFailure (Failure (msg.str().c_str(), name, file, line));                   
        }                                                                                              
    } catch( const TestException& e ) {                                                                
        ExceptionHandler::Handle(result_, e, name, file, line );                             
    } catch(...) {                                                                                     
        std::stringstream msg;                                                                         
        msg << "expected: '" << (expected) << "' but was: '" << (actual) << "'" << std::ends;       
        ExceptionHandler::Handle(result_, msg.str().c_str(), name, file, line);                   
    }                                                                                                  
}

template < typename Type1, typename Type2, typename Type3 >
void CheckClose( TestResult& result_, const Type1& expected, const Type2& actual, const Type3& epsilon, const char* file, unsigned int line, const char* name)
{
    try {                                                                                              
        if ((((expected)-(actual)) > (epsilon)) ||                                                    
            (((actual)-(expected)) > (epsilon))) {                                                    
            std::stringstream msg;                                                                     
            msg  << "expected: " << (expected) << " but was: " << (actual) << std::ends;            
            result_.AddFailure (Failure (msg.str().c_str(),  name, file, line));                
        }                                                                                              
    } catch( const TestException& e ) {                                                                
        ExceptionHandler::Handle(result_, e, name, file, line);                              
    } catch(...) {                                                                                     
        ExceptionHandler::Handle(result_, "", name, file, line);                 
    }
}

template < typename Type1, typename Type2, typename Type3 >
void CheckArrayClose( TestResult& result_, 
                     const Type1& expected, const Type2& actual, 
                     int count, const Type3& epsilon,  const char* file, unsigned int line, const char* name)
{
    try {
        bool ok = true;
        for (int i=0; i<count; ++i) 
        {
            if ( !(((((expected)[i]-(actual)[i]) <= epsilon) &&                         
                    ((actual)[i]-(expected)[i]) <= epsilon)) ) 
            {
                ok = false;
                break;
            }
        }

        if (!ok)
        {
            std::stringstream msg;                                                   
            msg  << "expected: [ ";                                               
            for (int j=0; j<(count); ++j) {                                          
                msg  << expected[j] << " ";                                     
            }                                                                        
            msg  << "] but was: [ ";                                              
            for (int j=0; j<(count); ++j) {                                          
                msg  << actual[j]  << " ";                                      
            }                                                                        
            msg  << "]" << std::ends;                                             
            result_.AddFailure (Failure (msg.str().c_str(),  name, file, line));
        }
    } catch( const TestException& e ) {                                                  
        ExceptionHandler::Handle(result_, e, name, file, line);                
    } catch(...) {                                                                       
        ExceptionHandler::Handle(result_, "", name, file, line);     
    }                                                                                    
}


template < typename Type1, typename Type2, typename Type3 >
void CheckArrayClose2D( TestResult& result_, 
                       const Type1& expected, const Type2& actual, 
                       int rows, int columns,
                       const Type3& epsilon,  const char* file, unsigned int line, const char* name)
{
    try 
    {
        bool ok = true;
        for (int i=0; ok && i<rows; ++i)
        {
            for (int j=0; ok && j<columns; ++j)
            {
                if ( !((((expected[i][j]-actual[i][j]) <= epsilon) &&
                         (actual[i][j]-expected[i][j]) <= epsilon)) ) 
                {
                    ok = false;
                    break;
                }
            }
        }

        if (!ok)
        {
            std::stringstream msg;
            msg.precision(4);
            msg << std::showpoint;
            msg << std::fixed;
            msg  << "expected: " << std::endl;
            msg << "[ ";                                               
            for (int i=0; i<rows; ++i) {                                          
                msg  << "[ ";
                for (int j=0; j<columns; ++j) {                                          
                    msg  << expected[i][j] << " ";                                     
                }
                msg  << "]";
                msg.width(5);
            }
            msg  << " ] but was:" << std::endl;
            msg << "[ ";                                              
            for (int i=0; i<rows; ++i) {                                          
                msg  << "[ ";
                for (int j=0; j<columns; ++j) {                                          
                    msg  << actual[i][j]  << " ";
                }
                msg  << "]";
                msg.width(5);
            }                                                                        
            msg  << " ]" << std::ends;                                             
            result_.AddFailure (Failure (msg.str().c_str(),  name, file, line));
        }
    }
    catch( const TestException& e ) 
    {
        ExceptionHandler::Handle(result_, e, name, file, line);                
    } 
    catch(...) 
    {
        ExceptionHandler::Handle(result_, "", name, file, line);     
    }   
}

}


#ifdef _MSC_VER
    #pragma warning (pop)
#endif

#endif
