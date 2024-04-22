

#include <iostream>
#include "Minerva/EngineStartup.h"
//Example main function to test the environment
int main() 
{
     Minerva::EngineStartup app;

    try {
        app.RunEngine();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}