
#ifndef TESTRESULT_H
#define TESTRESULT_H


class Failure;

class TestResult
{
public:
    TestResult ();
    virtual ~TestResult();

    virtual void TestWasRun ();
    virtual void StartTests ();
    virtual void AddFailure (const Failure & failure);
    virtual void EndTests ();

    int FailureCount() const;
    int TestCount() const;

protected:
    int m_failureCount;
    int m_testCount;
    long int m_startTime;
    float m_secondsElapsed;
};

#endif
