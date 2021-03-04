#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
#include <ctime>
namespace CityFlow{
    multiprocessor::multiprocessor()
    {
        Engine* engine = new Engine("./10_10_1/config_10_10.json", 1, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_2/config_10_10.json", 1, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_3/config_10_10.json", 1, this);
        engines.push_back(engine);
        engine = new Engine("./10_10_4/config_10_10.json", 1, this);
        engines.push_back(engine);
        std::cout << "end of initengines" << std::endl;
        initEngineRoad();
        std::cout << "end of initroads" << std::endl;
        for (size_t i = 0; i < engines.size(); ++i)
        {
            engines[i]->startThread();
        }
    }

    void multiprocessor::initEngineRoad()
    {
        // if ( engines[0]->getRoadNet().getRoadById("road_1_0_1")!=nullptr )
        // {
        //     std::cerr << engines[0]->getRoadNet().getRoadById("road_1_0_1")->getId() << std::endl;
        // }
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
        std::cerr << "pro next start" << std::endl;
        exchangeVehicle();
        std::cerr << "pro next end" << std::endl;
    }

    void multiprocessor::exchangeVehicle()
    {
        for (auto engine : engines)
        {
            for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
            {
                Vehicle oldVehicle = vehiclePair.first;
                // std::cerr << &oldVehicle << std::endl;
                // oldVehicle.getFlow();
                // std::cerr << "flow" << std::endl;
                // oldVehicle.getFlow()->vehicleTemplate;
                // std::cerr << "template" << std::endl;
                // oldVehicle.getId();
                // std::cerr << "id" << std::endl;
                // oldVehicle.getBufferEngine();
                // std::cerr << "engine" << std::endl;
                // Vehicle *vehicle = new Vehicle(oldVehicle.getFlow()->vehicleTemplate, oldVehicle.getId(), oldVehicle.getBufferEngine(), oldVehicle.getFlow());
                std::cerr << "start create" << std::endl;
                Vehicle *vehicle = new Vehicle(oldVehicle, oldVehicle.getId(), oldVehicle.getBufferEngine(), nullptr);
                std::cerr << "vehi created" << std::endl;
                vehicle->updateRoute();
                engine->pushVehicle(vehicle, false);
            }
            engine->clearChangeEnginePopBuffer();
        }
    }
}