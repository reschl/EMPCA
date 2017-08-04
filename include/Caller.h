#ifndef CALLER
#define CALLER

#include "Types.h"

class Caller {
    public:
        virtual ~Caller() {}
        virtual void callback(request rq) = 0;
};

#endif // CALLER

