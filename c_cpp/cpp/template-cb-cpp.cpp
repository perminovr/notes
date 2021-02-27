#include <iostream>
#include <thread>
#include <functional>

using namespace std;

class A {
public:
	void func(int a) {
		cout << "func: " << a << endl;
	}
};

class B {
public:
	template <class C>
	void setFunc(C *obj, void (C::*func)(int)) {
		using std::placeholders::_1;
		f = std::bind(func, obj, _1);
	}
	void call(int a) {
		f(a);
	}

private:
	std::function <void(int)> f;
};

int main()
{
	A a;
	B b;
	b.setFunc(&a, &A::func);
	b.call(3);
	return 0;
}