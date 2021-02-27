
#include <dlfcn.h>

int main()
{
	void *library_handler = dlopen("./libso_exmp.so", RTLD_LAZY);
	void (*printName)(void);
	printName = dlsym(library_handler, "printName");
	printName();
	return 0;
}
