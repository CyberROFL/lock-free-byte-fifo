#ifndef TESTRESULT_H
#define TESTRESULT_H

#include <iostream>

enum ErrorReason {
     ErrorInvalidData,
     ErrorMissingElements,
     ErrorAdditionalElements
};

class CTestResult
{
    bool        _errorFlag;
    ErrorReason _errorReason;

public:
    CTestResult() : _errorFlag(false) {}

    void SetError(ErrorReason reason)
    {
        _errorFlag   = true;
        _errorReason = reason;
    }

    void Print() const
    {
        if (!_errorFlag)
        {
            std::cout << "passed" << std::endl;
        }
        else
        {
            std::cout << "error (";

            switch (_errorReason)
            {
            case ErrorInvalidData:
                std::cout << "invalid data";
            break;

            case ErrorMissingElements:
                std::cout << "missing elements";
            break;

            case ErrorAdditionalElements:
                std::cout << "additional elements";
            break;

            default:
                std::cout << "unknown";
            break;
            }

            std::cout << ")" << std::endl;
        }
    }
};

#endif // TESTRESULT_H
