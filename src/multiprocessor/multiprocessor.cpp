#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
#include <ctime>
#include <time.h>

#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/cursorstreamwrapper.h"
#include "rapidjson/writer.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

namespace CityFlow{
    std::vector<Engine*> multiprocessor::engines = std::vector<Engine*>();
    multiprocessor::multiprocessor()
    {
        Engine* engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_3/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_1/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_4/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_2/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        // std::cout << "end of initengines" << std::endl;
        //initEngineRoad();

        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            multiprocessor::engines[i]->initId(i);
            multiprocessor::engines[i]->roadnet.initEnginePointer();
            multiprocessor::engines[i]->roadnet.initRoadPointer(engines);
            multiprocessor::engines[i]->startThread();
        }
        std::cout << "end of init" << std::endl;
    }

    void multiprocessor::engineNext(int i){
        multiprocessor::engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro()
    {
        std::vector<std::thread> threads1;
        for(size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            threads1.emplace_back(std::thread(&multiprocessor::engineNext,this,i));
        }
        for (size_t i = 0; i < threads1.size(); i++)
        {
            threads1[i].join();
        }
        // std::cout << "start exchangevehi" << std::endl;

        exchangeVehicle();
        // std::cout << "end of exchangevehi" << std::endl;

        std::vector<std::thread> threads2;
        for (size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            threads2.emplace_back(std::thread(&multiprocessor::updateHistory,this,i));
        }
        for (size_t i = 0; i < threads2.size(); i++)
        {
            threads2[i].join();
        }
        // std::cout << "nextsteppro end" << std::endl;
    }

    void multiprocessor::updateHistory(int i)
    {
        multiprocessor::engines[i]->updateHistory();
    }

    void multiprocessor::exchangeVehicle()
    {
        // std::vector<std::thread> threads;
        // for (auto engine : multiprocessor::engines)
        // {
        //     for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
        //     {
        //         threads.emplace_back(std::thread(&multiprocessor::generateVehicle, this,vehiclePair.first));
        //     }
        // }
        // for (size_t i = 0; i < threads.size(); i++)
        // {
        //     threads[i].join();
        // }

        for (auto engine : multiprocessor::engines)
        {
            for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
            {
                generateVehicle(vehiclePair.first);
            }
        }

        for (auto engine : multiprocessor::engines)
        {
            engine->clearChangeEnginePopBuffer();
        }
    }

    void multiprocessor::generateVehicle(Vehicle oldVehicle)
    {
        Engine* bufferEngine = oldVehicle.getBufferEngine();
        Vehicle *vehicle = new Vehicle(oldVehicle, oldVehicle.getId() + "_CE", bufferEngine, nullptr);
        // std::cerr << "vehi created" << std::endl;

        Road * belongRoad = oldVehicle.getChangedDrivable()->getBelongRoad();
        vehicle->getControllerInfo()->router.resetAnchorPoints(belongRoad, bufferEngine->getId());
        // std::cerr << "route reset" << std::endl;

        int priority = vehicle->getPriority();
        while (bufferEngine->checkPriority(priority)) priority = bufferEngine->rnd();
        vehicle->setPriority(priority);
        bufferEngine->pushVehicle(vehicle, false);
        bufferEngine->activeVehicleCount++;
        vehicle->updateRoute();
        // std::cerr << "route update" << std::endl;

        vehicle->setFirstDrivable();
        Lane *lane = (Lane *)(vehicle->getChangedDrivable());
        Vehicle * tail = lane->getLastVehicle();
        lane->pushVehicle(vehicle);
        vehicle->updateLeaderAndGap(tail);
        // std::cerr << "leaderandgap update" << std::endl;

        vehicle->update();
        vehicle->clearSignal();
    }
}