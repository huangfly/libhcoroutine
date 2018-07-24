#include "coroutine.h"
#include<functional>
#include <iostream>
int main() {
	auto func1 = [](void) {
		std::cout << "1" << std::endl;
		Yeild();
		std::cout << "3" << std::endl;};
	auto func2 = [](void) {
		std::cout << "2" << std::endl;
		Yeild();
		std::cout << "4" << std::endl; };
	AddTask(func1);
	AddTask(func2);
	Run();
	return 0;
}