#ifndef FOOINTERFACE_H
#define FOOINTERFACE_H

#include <string>

class FooInterface {
public:
        virtual ~FooInterface() {}
        virtual std::string getArbitraryString() = 0;
};

#endif // FOOINTERFACE_H
