
#ifndef FAILURE_H
#define FAILURE_H

#include <iosfwd>


class Failure
{
public:
    Failure (const char* condition, const char* testName, 
             const char* fileName, int lineNumber);

    const char* Condition() const;
	const char* TestName() const;
	const char* FileName() const;
	int LineNumber() const;

private:
	friend std::ostream& operator<< (std::ostream& stream, const Failure& failure);
	friend std::wostream& operator<< (std::wostream& stream, const Failure& failure);
    
    const char* m_condition;
    const char* m_testName;
    const char* m_fileName;
    int m_lineNumber;
};


#endif

