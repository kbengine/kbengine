#include "Failure.h"

#include <string>
#include <iostream>


Failure::Failure (const char* condition, const char* testName, 
                  const char* fileName, int lineNumber) 
    : m_condition (condition)
    , m_testName (testName)
    , m_fileName (fileName)
    , m_lineNumber (lineNumber)
{
}

const char* Failure::Condition() const
{
    return m_condition;
}


const char* Failure::TestName() const
{
	return m_testName;
}


const char* Failure::FileName() const
{
	return m_fileName;
}


int Failure::LineNumber() const
{
	return m_lineNumber;
}


std::ostream& operator<< (std::ostream& stream, const Failure& failure)
{
	stream 
		<< failure.m_fileName
		<< "(" << failure.m_lineNumber << "): "
		<< "Failure in " << failure.m_testName << ": \"" << failure.m_condition << "\" " 
		<< std::endl;

	return stream;
}

std::wostream& operator<< (std::wostream& stream, const Failure& failure)
{
	stream 
		<< failure.m_fileName
		<< L"(" << failure.m_lineNumber << L"): "
		<< L"Failure in " << failure.m_testName << L": \"" << failure.m_condition << L"\" " 
		<< std::endl;

	return stream;
}

