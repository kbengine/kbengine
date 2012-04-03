#ifndef MOCKTESTRESULT_H_
#define MOCKTESTRESULT_H_

#include "../TestResult.h"
#include "../Failure.h"
#include <sstream>

class MockTestResult : public TestResult
{
public:
    void AddFailure( const Failure & failure ) 
    {
        TestResult::AddFailure(failure);
        std::stringstream stream;
        stream << failure << std::endl;
        msg = stream.str();
    }
    
    std::string msg;
};


#endif

