
#include "so_exmp.h"



int main()
{
	A *aa = new A();
	aa->print();
	delete aa;
	A a;
	a.print();
	return 0;
}
