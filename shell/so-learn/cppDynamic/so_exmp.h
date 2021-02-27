
#include <string>

class A {
public:
	A();
	/* use virtual otherwise linker will try to perform static linkage */
	virtual ~A() = default;
	virtual void print();
private:
	std::string str;
};
