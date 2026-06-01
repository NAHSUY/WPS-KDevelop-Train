#ifndef KX_PERSON_H
#define KX_PERSON_H

#include <string>

class KxPerson
{
public:
	KxPerson() = default;
	KxPerson(const std::string& name);
	~KxPerson();
	virtual void showName() const;
private:
	std::string m_name;
};

#endif // KX_PERSON_H