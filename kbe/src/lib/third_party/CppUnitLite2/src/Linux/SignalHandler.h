#ifndef SIGNALHANDLER_H_
#define SIGNALHANDLER_H_

// Scoped object that takes care of setting up the correct signal handlers,
// removing them, and triggering a C++ exception if a signal is found.

class SignalHandler
{
public:
    SignalHandler(bool bCatchSystemExceptions);
    ~SignalHandler();
    
private:
    bool m_bCatchSystemExceptions;
    
};


#endif

