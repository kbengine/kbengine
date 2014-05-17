#ifndef cppunitlite2_mockhelpers_h
#define cppunitlite2_mockhelpers_h

template < typename Type >
class MockInstanceTracker
{
public:
    MockInstanceTracker( const MockInstanceTracker& )
    {
        ++s_instanceCount;
    }

    ~MockInstanceTracker()
    {
        --s_instanceCount;
    }
    
    static int s_instanceCount;
    
protected:
    MockInstanceTracker()
    {
        ++s_instanceCount;
    }
};

template <typename Type>
int MockInstanceTracker<Type>::s_instanceCount = 0;

#endif
