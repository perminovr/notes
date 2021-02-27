
#include "so_exmp.h"
#include <iostream>


extern "C" A* create_object()
{
	return new A;
}

extern "C" void destroy_object( A* object )
{
	delete object;
}


A::A()
{
	this->str = "data";
}


void A::print()
{
	std::cout << this->str << std::endl;
}
