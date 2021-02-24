#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
namespace CityFlow{
    multiprocessor::multiprocessor()
    {
        Engine* engine = new Engine("./10_10_1/config_10_10.json", 10, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_2/config_10_10.json", 10, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_3/config_10_10.json", 10, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_4/config_10_10.json", 10, this);
        engines.push_back(engine);
        std::cout << "end of init" << std::endl;
    }

    void multiprocessor::engineNext(int i){
        engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro()
    {
        std::vector<std::thread> threads;
        for(size_t i = 0; i < engines.size(); i++)
        {
            threads.emplace_back(std::thread(&multiprocessor::engineNext,this,i));
        }
        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }
        exchangeVehicle();
    }

    void multiprocessor::exchangeVehicle()
    {
        for (auto &engine : engines)
        {
            for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
            {
                Vehicle *vehicle = vehiclePair.first;
                Drivable *drivable = vehicle->getChangedDrivable();
                Vehicle * tail = drivable->getLastVehicle();
                if (drivable != nullptr)
                {
                    drivable->pushVehicle(vehicle);
                    vehicle->updateLeaderAndGap(tail);
                    vehicle->update();
                    vehicle->clearSignal();
                }
            }
            engine->clearChangeEnginePopBuffer();
        }
    }
}