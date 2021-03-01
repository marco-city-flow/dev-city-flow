#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
namespace CityFlow{
    multiprocessor::multiprocessor()
    {
        Engine* engine = new Engine("./10_10_1/config_10_10.json", 1, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_2/config_10_10.json", 10, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_3/config_10_10.json", 10, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_4/config_10_10.json", 10, this);
        engines.push_back(engine);
        std::cout << "end of initengines" << std::endl;
        initEngineRoad();
        std::cout << "end of initroads" << std::endl;
    }

    void multiprocessor::initEngineRoad()
    {
        for (size_t j = 0; j < engines.size(); ++j)
        {
            std::vector<Road> roads = engines[j]->getRoadNet().getRoads();
            for (size_t i = 0; i < roads.size(); i++)
            {
                std::string id = roads[i].getId();
                std::string col,row;
                if (id.substr(6,1) >= "0" && id.substr(6,1) <= "9")
                {
                    col = id.substr(5,2);
                    if (id.substr(9,1) >= "0" && id.substr(9,1) <= "9")
                    {
                        row = id.substr(8,2);
                    }
                    else
                    {
                        row = id.substr(8,1);
                    }
                }
                else
                {
                    col = id.substr(5,1);
                    if (id.substr(8,1) >= "0" && id.substr(8,1) <= "9")
                    {
                        row = id.substr(7,2);
                    }
                    else
                    {
                        row = id.substr(7,1);
                    }
                }

                if (atoi(col.c_str())>=6 && atoi(row.c_str())>=6)
                {
                    roads[i].initEngine(engines[0]);
                }
                if (atoi(col.c_str())<6 && atoi(row.c_str())>=6)
                {
                    roads[i].initEngine(engines[1]);
                }
                if (atoi(col.c_str())<6 && atoi(row.c_str())<6)
                {
                    roads[i].initEngine(engines[2]);
                }
                if (atoi(col.c_str())>=6 && atoi(row.c_str())<6)
                {
                    roads[i].initEngine(engines[3]);
                }
            }
        }
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
                Drivable *predrivable = vehicle->getCurDrivable();
                Vehicle *tail = drivable->getLastVehicle();
                Vehicle *head = predrivable->getFirstVehicle();
                if (drivable != nullptr)
                {
                    drivable->pushVehicle(vehicle);
                    vehicle->updateLeaderAndGap(tail);
                    head->updateLeaderAndGap(vehicle);
                    vehicle->update();
                    vehicle->clearSignal();
                }
            }
            engine->clearChangeEnginePopBuffer();
        }
    }
}