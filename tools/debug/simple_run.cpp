#include "engine/engine.h"
#include "utility/optionparser.h"
#include "multiprocessor/multiprocessor.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace CityFlow;

int main(int argc, char const *argv[]) {
    // Engine engine1("./../../build/10_10_1/config_10_10.json",1);
    // engine1.nextStep();
    // return 0;
    multiprocessor multi("gfdh");
    for (size_t i = 0; i < 40; i++)
    {
        multi.nextStepPro_F();
        std::cout << i << std::endl;
    }
}