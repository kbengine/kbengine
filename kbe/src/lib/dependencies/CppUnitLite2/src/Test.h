#ifndef TEST_H
#define TEST_H

#include "ExceptionHandler.h"
#include "TestException.h"
#include "TestMacros.h"
#include "TestResult.h"
#include "Failure.h"

// #define DONT_CHECK_ASSERTS     1


class Test
{
public:
    Test (const char * testName,
          const char * filename, int linenumber);
    virtual ~Test();

    virtual void Run (TestResult& result );

protected:
    virtual void RunTest (TestResult& result) = 0;

    const char * m_name;
    const char * m_filename;
    const int m_linenumber;

private:
    Test(const Test &);
    Test& operator=(const Test &);

};



class TestRegistrar
{
public:
    TestRegistrar(Test * test);

};


#define TEST(test_name)                                                                 \
    class test_name##Test : public Test                                                 \
    {                                                                                   \
        public:                                                                         \
            test_name##Test () : Test (#test_name "Test", __FILE__, __LINE__){}   \
        protected:                                                                      \
            virtual void RunTest (TestResult& result_);                                 \
    } test_name##Instance;                                                              \
    TestRegistrar test_name##_registrar (&test_name##Instance);                         \
    void test_name##Test::RunTest (TestResult& result_)


// Test with fixture
#define TEST_F(fixture, test_name)                                                      \
    struct fixture##test_name : public fixture {                                        \
        fixture##test_name(const char * name_) : m_name(name_) {}                       \
        void test_name(TestResult& result_);                                            \
        const char * m_name;                                                            \
    };                                                                                  \
    class fixture##test_name##Test : public Test                                        \
    {                                                                                   \
        public:                                                                         \
            fixture##test_name##Test ()                                                 \
                : Test (#test_name "Test", __FILE__, __LINE__) {}                 \
        protected:                                                                      \
            virtual void RunTest (TestResult& result_);                                 \
    } fixture##test_name##Instance;                                                     \
    TestRegistrar fixture##test_name##_registrar (&fixture##test_name##Instance);       \
    void fixture##test_name##Test::RunTest (TestResult& result_) {                      \
        fixture##test_name mt(m_name);                                                  \
        mt.test_name(result_);                                                          \
    }                                                                                   \
    void fixture ## test_name::test_name(TestResult& result_)


// Test with fixture with construction parameters
#define TEST_FP(fixture, fixture_construction, test_name)                                                      \
    struct fixture##test_name : public fixture {                                        \
        fixture##test_name(const char * name_) : fixture_construction, m_name(name_) {}                       \
        void test_name(TestResult& result_);                                            \
        const char * m_name;                                                            \
    };                                                                                  \
    class fixture##test_name##Test : public Test                                        \
    {                                                                                   \
        public:                                                                         \
            fixture##test_name##Test ()                                                 \
                : Test (#test_name "Test", __FILE__, __LINE__) {}                 \
        protected:                                                                      \
            virtual void RunTest (TestResult& result_);                                 \
    } fixture##test_name##Instance;                                                     \
    TestRegistrar fixture##test_name##_registrar (&fixture##test_name##Instance);       \
    void fixture##test_name##Test::RunTest (TestResult& result_) {                      \
        fixture##test_name mt(m_name);                                                  \
        mt.test_name(result_);                                                          \
    }                                                                                   \
    void fixture ## test_name::test_name(TestResult& result_)


#define CHECK(condition)                                                               \
    do {                                                                               \
        try {                                                                          \
            if (!(condition))                                                          \
				result_.AddFailure (Failure (#condition, m_name, __FILE__, __LINE__)); \
        } catch( const TestException& e ) {											   \
            ExceptionHandler::Handle(result_, e, m_name, __FILE__, __LINE__); 	       \
        } catch(...) {                                                                 \
            ExceptionHandler::Handle(result_, #condition, m_name, __FILE__, __LINE__); \
        }                                                                              \
    } while (0)


#define CHECK_EQUAL(expected,actual)                                					 \
    do                                                              					 \
    {                                                               					 \
        cppunitlite::CheckEqual( result_, expected, actual, __FILE__, __LINE__, m_name );\
    } while( 0 )                                                    					 \

#define CHECK_CLOSE(expected,actual,kEpsilon)													  \
    do {																						  \
        cppunitlite::CheckClose( result_, expected, actual, kEpsilon, __FILE__, __LINE__, m_name );\
    } while(0)


#define CHECK_ARRAY_CLOSE(expected,actual,count,kEpsilon)                               	 \
    do {                                                                                 	 \
		cppunitlite::CheckArrayClose( result_, expected, actual, count, kEpsilon, __FILE__, __LINE__, m_name ); \
    } while(0)

#define CHECK_ARRAY2D_CLOSE(expected,actual,rows,columns,kEpsilon)                           \
	do {                                                                                 	 \
		cppunitlite::CheckArrayClose2D( result_, expected, actual, rows, columns, kEpsilon, __FILE__, __LINE__, m_name ); \
	} while(0)



#if defined DONT_CHECK_ASSERTS

#define	CHECK_ASSERT(action)																	\
	do { 																						\
		&result_;																				\
    } while (0)

#else

#define	CHECK_ASSERT(action)																	\
	do { 																						\
		bool bExceptionCaught =	false; 															\
		try	{ 																					\
			action;																				\
		} catch(const TestException& ) {														\
			bExceptionCaught = true; 															\
		} 																						\
		catch (...) {																			\
			ExceptionHandler::Handle(result_, #action, m_name, __FILE__, __LINE__);				\
		}																						\
		if (!bExceptionCaught) 																	\
			result_.AddFailure (Failure	("No exception detected", m_name, __FILE__,	__LINE__));	\
    } while (0)

#endif

/**
 *  BigWorld additions to the CppUnitLite2 interface, to make it more like
 *  vanilla cppunit.  Notable difference between these and the other macros is
 *  that these will terminate the current TEST() block (like a normal assertion
 *  does) rather than continuing on with the subsequent tests.
 */

#define FAIL( message )														\
    do {																	\
        try {																\
            result_.AddFailure(												\
				Failure (message, m_name, __FILE__, __LINE__));				\
		}catch( const TestException& e ){									\
			ExceptionHandler::Handle(										\
				result_, e, m_name, __FILE__, __LINE__);					\
        } catch(...) {														\
            ExceptionHandler::Handle(										\
				result_, message, m_name, __FILE__, __LINE__);				\
        }																	\
																			\
		return;																\
																			\
    } while (0)																\


#define ASSERT_WITH_MESSAGE( condition, message )							\
    do {																	\
        try {																\
            if (!(condition))												\
			{																\
				result_.AddFailure(											\
					Failure (message, m_name, __FILE__, __LINE__));			\
			}																\
		}catch( const TestException& e ){									\
			ExceptionHandler::Handle(										\
				result_, e, m_name, __FILE__, __LINE__);					\
        } catch(...) {														\
            ExceptionHandler::Handle(										\
				result_, message, m_name, __FILE__, __LINE__);				\
        }																	\
																			\
		if (!(condition))													\
		{																	\
			return;															\
		}																	\
																			\
    } while (0)																\


#endif
