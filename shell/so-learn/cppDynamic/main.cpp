
#include "so_exmp.h"
#include <dlfcn.h>


int main()
{
	void* handle = dlopen("./libso_exmp.so", RTLD_LAZY);
	A* (*create)();
  	void (*destroy)(A*);
  	
  	create = (A* (*)())dlsym(handle, "create_object");
  	destroy = (void (*)(A*))dlsym(handle, "destroy_object");
  	
  	A *a = (A*)create();
  	a->print();
  	destroy(a);
  	
  	A *aa = (A*)create();
  	aa->print();
  	delete aa;
  
	return 0;
}
