#include "engine/engine.h"
#include "utility/optionparser.h"

#include <string>
#include <iostream>
#include <cstdlib>
#include <ctime>

using namespace CityFlow;

int main(int argc, char const *argv[]) {
    Engine engine("./nanjingmega/nanjingmegaconfig.json", 1);
    engine.nextStep();
    return 0;
}