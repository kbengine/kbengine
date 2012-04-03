#include "../CppUnitLite2.h"
#include "../TestResultStdErr.h"

int main()
{     
    TestResultStdErr result;
    TestRegistry::Instance().Run(result);
    return (result.FailureCount());
}

