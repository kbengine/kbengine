

#ifndef TESTRESULTDEBUGOUT_WIN32_H
#define TESTRESULTDEBUGOUT_WIN32_H

#include "../TestResult.h"


class TestResultDebugOut : public TestResult
{
public:
    virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();
};


#endif
