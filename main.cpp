#include "first_app.hpp"

//std
#include<cstdlib>
#include<iostream>
#include<stdexcept>

int main() 
{
	for (int i = 0; i < 10; i++) 
	{
		lve::FirstApp app{};

		try
		{
			app.run();
		}
		catch (const std::exception& e)
		{
			std::cerr << e.what() << "\n";
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}
	//test
	//test2
	//test3
}
