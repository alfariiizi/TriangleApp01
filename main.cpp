#include <exception>
#include <iostream>

#include "HelloTriangleApp.h"

int main()
{
    HelloTriangleApp app;

    try{
        app.Run();
    }
    catch( std::exception& e )
    {
        std::cout << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}