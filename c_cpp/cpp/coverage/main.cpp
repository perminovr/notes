#include <iostream>

using namespace std;

class A {
public:
	A(){}
	~A(){}

	int a;

	void func() {
		a++;
	}

	void print() {
		cout << a << endl;
	}
};

int main()
{
	A a;

	a.a = 2;

	a.print();

	for (int i = 0; i< 10; ++i) {
		a.func();
	}

	a.print();

	return 0;
}