
#ifndef __OgreApplication_h_
#define __OgreApplication_h_

#include "BaseApplication.h"

class OgreApplication : public BaseApplication
{
public:
    OgreApplication(void);
    virtual ~OgreApplication(void);

protected:
    virtual void createScene(void);
};

#endif // #ifndef __OgreApplication_h_
