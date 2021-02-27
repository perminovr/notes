
#include "nlwrapper.h"

#include <iostream>

using namespace std;

int main()
{
	NlSock sk;
	RtNlRoute::delWithoutSrc(sk);
	cout << endl;
	return 0;
}