//============================================================================
// Name        : tz_stl_auto.cpp
// Author      : 
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include "set_tz"
#include <stdlib.h>
#include <algorithm>
#include <typeinfo>
using namespace std;
using namespace tz;

#include <cxxabi.h>


char* demangle(const char *demangled)
{
	int status;
	return abi::__cxa_demangle(demangled, 0, 0, &status);
}

void main2();

int main() {
	main2();
	return 0;
}

void main2(){
	cout << "!!!Hello World!!!" << endl; // prints !!!Hello World!!!
	tz::multiset<int>& s(*new tz::multiset<int>);
	std::cout << __LINE__ << std::endl;
	std::cout << __LINE__ << std::endl;
	for(int i = 0; i < 10 ; i++){
		std::cout << __LINE__ << std::endl;
		s.insert(rand());
		std::cout << __LINE__ << std::endl;
	}
	std::cout << __LINE__ << std::endl;
	/*std::for_each(s.begin(), s.end(), [](int x) {
		std::cout << x << std::endl;
	});*/
	std::cout << "it_test" << std::endl;
	multiset<int>::iterator it = s.begin(), it2, it3, it4, it5;
	++it;
	it5 = it4 = it2 = it3 = it;
	for(multiset<int>::iterator it = s.begin(); it != s.end() ; ++it){
		std::cout << *it << std::endl;
		std::cout << *(void**)&it << ";" << it._M_node->_TZ_refCount << std::endl;
	}
	std::cout << __LINE__ << std::endl;
	std::cout << *s.end() << std::endl;
	std::cout << __LINE__ << std::endl;
	std::cout << demangle(typeid(s.end()).name()) << std::endl;
	std::cout << "adr:" << &s << std::endl;
	std::cout << "ptr_test" << std::endl;
	std::cout << "1:" << *(void**)&it << ":" << *it << std::endl;
	++it;
	std::cout << "2:" << *(void**)&it << ":" << *it << std::endl;
	++it;
	std::cout << "3:" << *(void**)&it << ":" << *it << std::endl;
	++it;
	std::cout << "4:" << *(void**)&it << ":" << *it << std::endl;
	++it;
	std::cout << "5:" << *(void**)&it << ":" << *it << std::endl;
	std::cout << "ptr_test2" << std::endl;
	it = s.begin();
	it2 = it3 = it;
	it5 = it4 = s.end();
	delete &s;
	std::cout << "1:" << *(void**)&it << ":" << *it << std::endl;
	--it;
	std::cout << "2:" << *(void**)&it << ":" << *it << std::endl;
	--it;
	std::cout << "3:" << *(void**)&it5 << ":" << &*it5 << std::endl;
	--it5;
	std::cout << "4:" << *(void**)&it5 << ":" << &*it5 << std::endl;
	--it5;
	std::cout << "5:" << *(void**)&it5 << ":" << &*it5 << std::endl;
}
