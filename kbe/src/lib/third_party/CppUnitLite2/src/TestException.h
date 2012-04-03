#ifndef TEST_EXCEPTION_H_
#define TEST_EXCEPTION_H_

class TestException
{
public:
    TestException( const char* file, int line, const char* message );
    
    const char* file;
    const int line;
    const char* message;
};


#endif
