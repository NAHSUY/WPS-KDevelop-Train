#ifndef PERSON_H
#define PERSON_H

#include <string>

class Person {
public:
    Person(const std::string& name, int age);
    std::string getName() const;
    int getAge() const;

private:
    std::string m_name;
    int m_age = 0;
};

#endif // PERSON_H
