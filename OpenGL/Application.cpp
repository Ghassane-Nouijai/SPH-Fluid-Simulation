#include "Application.h"

int main()
{
	try
	{
		Application app;
		app.Run();
	}
	catch (const std::exception& e)
	{
		std::cerr << "[Fatal] " << e.what() << '\n';
		return -1;
	}
	return 0;
}