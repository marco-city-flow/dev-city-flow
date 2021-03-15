#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
// #include <ctime>
#include <time.h>

namespace CityFlow{
    std::vector<Engine*> multiprocessor::engines = std::vector<Engine*>();
    multiprocessor::multiprocessor()
    {
        Engine* engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_1/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_2/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_3/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        engine = new Engine("/home/zhj/Desktop/CityFlow/build/10_10_4/config_10_10.json", 6, this);
        multiprocessor::engines.push_back(engine);
        // std::cout << "end of initengines" << std::endl;
        //initEngineRoad();
        // std::cout << "end of initroads" << std::endl;

        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            multiprocessor::engines[i]->roadnet.initEnginePointer();
            multiprocessor::engines[i]->startThread();
        }
    }

    //void multiprocessor::initEngineRoad(){
        // if ( engines[0]->getRoadNet().getRoadById("road_1_0_1")!=nullptr )
        // {
        //     std::cerr << engines[0]->getRoadNet().getRoadById("road_1_0_1")->getId() << std::endl;
        // }

        /* ------------------------------
        for (size_t j = 0; j < engines.size(); ++j)
        {
            std::vector<Road> &roads = (engines[j])->roadnet.getRoads();
            //std::cout << "end of roads" << std::endl;

            for (size_t i = 0; i < roads.size(); i++)
            {
                std::string id =  (roads[i]).getId();
                //std::cout << id << std::endl;

                std::string col,row;
                int coli,rowi,diri;
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
                //std::cout << "end of roads1" << std::endl;

                coli = atoi(col.c_str());
                rowi = atoi(row.c_str());
                diri = atoi(id.substr(id.size()-1).c_str());
                if (coli>=6 && rowi>=6)
                {
                    roads[i].initEngine(engines[0]);
                }
                if (coli<6 && rowi>=6)
                {
                    roads[i].initEngine(engines[1]);
                }
                if (coli<6 && rowi<6)
                {
                    roads[i].initEngine(engines[2]);
                }
                if (coli>=6 && rowi<6)
                {
                    roads[i].initEngine(engines[3]);
                }

                if (coli==6 && rowi>=6 && diri==2)
                {
                    roads[i].initEngine(engines[0], engines[1]);
                }
                if (coli==6 && rowi<6 && diri==2)
                {
                    roads[i].initEngine(engines[3], engines[2]);
                }
                if (coli==5 && rowi>=6 && diri==0)
                {
                    roads[i].initEngine(engines[1], engines[0]);
                }
                if (coli==5 && rowi<6 && diri==0)
                {
                    roads[i].initEngine(engines[2], engines[3]);
                }

                if (rowi==6 && coli>=6 && diri==3)
                {
                    roads[i].initEngine(engines[0], engines[3]);
                }
                if (rowi==6 && coli<6 && diri==3)
                {
                    roads[i].initEngine(engines[1], engines[2]);
                }
                if (rowi==5 && coli>=6 && diri==1)
                {
                    roads[i].initEngine(engines[3], engines[0]);
                }
                if (rowi==5 && coli<6 && diri==1)
                {
                    roads[i].initEngine(engines[2], engines[1]);
                }
                //std::cout << "end of roads2" << std::endl;

            }
        }
        */
    //}

    void multiprocessor::engineNext(int i){
        multiprocessor::engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro()
    {
        clock_t start, now;
        start = clock();
        std::vector<std::thread> threads1;
        for(size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            threads1.emplace_back(std::thread(&multiprocessor::engineNext,this,i));
        }
        for (size_t i = 0; i < threads1.size(); i++)
        {
            threads1[i].join();
        }
        now = clock();
        // std::cout << "engineNext:" << now - start << std::endl;

        start = clock();
        exchangeVehicle();
        now = clock();
        std::cout << "exchangeVehicle:" << now - start << std::endl;

        start = clock();
        std::vector<std::thread> threads2;
        for (size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            threads2.emplace_back(std::thread(&multiprocessor::updateHistory,this,i));
        }
        for (size_t i = 0; i < threads2.size(); i++)
        {
            threads2[i].join();
        }
        now = clock();
        // std::cout << "updateHistory:" << now - start << std::endl;
    }

    void multiprocessor::updateHistory(int i)
    {
        multiprocessor::engines[i]->updateHistory();
    }

    void multiprocessor::exchangeVehicle()
    {
        std::vector<std::thread> threads;
        for (auto engine : multiprocessor::engines)
        {
            for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
            {
                threads.emplace_back(std::thread(&multiprocessor::generateVehicle, this,vehiclePair.first));
            }
        }
        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }

        // for (auto engine : multiprocessor::engines)
        // {
        //     for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
        //     {
        //         generateVehicle(vehiclePair.first);
        //     }
        // }

        for (auto engine : multiprocessor::engines)
        {
            engine->clearChangeEnginePopBuffer();
        }
    }

    void multiprocessor::generateVehicle(Vehicle oldVehicle)
    {
        clock_t start, now;
        start = clock();
        Engine* bufferEngine = oldVehicle.getBufferEngine();
        Vehicle *vehicle = new Vehicle(oldVehicle, oldVehicle.getId() + "_CE", bufferEngine, nullptr);
        now = clock();
        std::cerr << "vehi created" << now - start << std::endl;

        Road * belongRoad = oldVehicle.getChangedDrivable()->getBelongRoad();
        start = clock();
        vehicle->getControllerInfo()->router.resetAnchorPoints(belongRoad, bufferEngine);
        now = clock();
        std::cerr << "route reset" << now - start << std::endl;

        start = clock();
        int priority = vehicle->getPriority();
        while (bufferEngine->checkPriority(priority)) priority = bufferEngine->rnd();
        vehicle->setPriority(priority);
        bufferEngine->pushVehicle(vehicle, false);
        bufferEngine->activeVehicleCount++;
        // vehicle->updateRoute();
        now = clock();
        std::cerr << "route update" << now - start << std::endl;

        start = clock();
        vehicle->setFirstDrivable();
        Lane *lane = (Lane *)(vehicle->getChangedDrivable());
        Vehicle * tail = lane->getLastVehicle();
        lane->pushVehicle(vehicle);
        vehicle->updateLeaderAndGap(tail);
        now = clock();
        std::cerr << "leaderandgap update" << now - start << std::endl;

        vehicle->update();
        vehicle->clearSignal();
    }
}