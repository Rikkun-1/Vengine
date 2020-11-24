#include <iostream>
#include <stdexcept>
#include <cstdlib> // ������� EXIT_SUCCESS � EXIT_FAILURE

#include "Renderer.h"

using std::cout;
using std::cerr;
using std::endl;

int main() 
{
	Renderer app;

	try 
	{
		app.run();
	}
	catch(const std::exception &e) 
	{
		cerr << e.what() << endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}