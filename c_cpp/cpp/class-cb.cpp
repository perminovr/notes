#include <iostream>
#include <functional>

using namespace std;

class A {
public:
	void func(int a) {
		cout << "func: " << a << endl;
	}

	function <void(int)> cb;
};

int main()
{
	A a;
	using std::placeholders::_1;
	a.cb = bind(&A::func, &a, _1);

	a.cb(0);
}