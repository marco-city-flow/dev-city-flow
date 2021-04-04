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
    multiprocessor::multiprocessor(const std::string &configFile)
    {
        std::string cconfigFile = "/home/zhj/Desktop/CityFlow/build/10_10_m/config_10_10.json";
        loadFromConfig(cconfigFile);

        // std::cout << "end of initengines" << std::endl;

        // std::vector<std::thread> threads;
        // for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        // {
        //     threads.emplace_back(std::thread(&multiprocessor::initEngines,this,i));
        // }
        // for (size_t i = 0; i < threads.size(); i++)
        // {
        //     threads[i].join();
        // }
        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            initEngines(i);
        }
        std::cout << "end of init" << std::endl;
    }

    void multiprocessor::initEngines(int i)
    {
        multiprocessor::engines[i]->initId(i);
        multiprocessor::engines[i]->roadnet.initEnginePointer();
        multiprocessor::engines[i]->roadnet.initRoadPointer(engines);
        multiprocessor::engines[i]->startThread();
        std::cout << "init" << i << std::endl;
    }

    bool multiprocessor::loadFromConfig(std::string jsonFileName)   // load engines from an all-in-one config
    {
        rapidjson::Document document;
        if (!readJsonFromFile(jsonFileName, document)) {
            std::cerr << "cannot open roadnet file" << std::endl;
            return false;
        }
        std::list<std::string> path;
        if (!document.IsObject())
            throw JsonTypeError("roadnet config file", "object");
        try {
            const rapidjson::Value &engineConfigs = getJsonMemberArray("engines", document);

            //  build mapping
            //engines.resize(engineConfigs.Size());
            for (rapidjson::SizeType i = 0; i < engineConfigs.Size(); i++) {
                const auto &curEngineConfig = engineConfigs[i];
                if (!curEngineConfig.IsObject()) {
                    throw JsonTypeError("engineConfig", "object");
                    return false;
                }
                std::string path_t = getJsonMember<const char*>("engineDir", curEngineConfig);
                path_t += getJsonMember<const char*>("configFile", curEngineConfig);
                std::cerr << path_t << std::endl;
                Engine *engine = new Engine(path_t, 1, this);
                multiprocessor::engines.push_back(engine);
            }
        }catch (const JsonFormatError &e) {
            std::cerr << "Error occurred when reading the roadnet file: " << std::endl;
            for (const auto &node : path) {
                std::cerr << "/" << node;
            }
            std::cerr << " " << e.what() << std::endl;
            return false;
        }
        return true;
    }

    void multiprocessor::engineNext(int i){
        multiprocessor::engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro_F(size_t step){
        for (size_t j = 0; j < step; j++)
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

            if (j % 5 == 0)
            {
                // std::vector<std::thread> threads2;
                // for (size_t i = 0; i < multiprocessor::engines.size(); i++)
                // {
                //     threads2.emplace_back(std::thread(&multiprocessor::syncFlow,this,i));
                // }
                // for (size_t i = 0; i < threads2.size(); i++)
                // {
                //     threads2[i].join();
                // }
                for (size_t i = 0; i < multiprocessor::engines.size(); i++)
                {
                    syncFlow(i);
                }
            }
            std::cerr << j << std::endl;
        }
    }

    void multiprocessor::syncFlow(int i){
        engines[i]->syncFlow(i);
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
        std::vector<std::thread> threads1;
        for (auto engine : multiprocessor::engines)
        {
            for (auto &vehiclePair : engine->getChangeEnginePopBuffer())
            {
                threads1.emplace_back(std::thread(&multiprocessor::generateVehicle, this,vehiclePair.first));
            }
        }
        for (size_t i = 0; i < threads1.size(); i++)
        {
            threads1[i].join();
        }

        for (size_t i = 0; i < vehiclePushBuffer.size(); i++)
        {
            Vehicle* vehicle = vehiclePushBuffer[i];
            Engine* bufferEngine = vehicle->getBufferEngine();
            int priority = vehicle->getPriority();
            while (bufferEngine->checkPriority(priority)) priority = bufferEngine->rnd();
            vehicle->setPriority(priority);
            bufferEngine->pushVehicle(vehicle, false);
        }

        std::vector<std::thread> threads2;
        for (size_t i = 0; i < vehiclePushBuffer.size(); i++)
        {
            threads2.emplace_back(std::thread(&multiprocessor::pushInEngine, this, i));
        }
        for (size_t i = 0; i < threads2.size(); i++)
        {
            threads2[i].join();
        }

        for (auto engine : multiprocessor::engines)
        {
            engine->clearChangeEnginePopBuffer();
        }

        vehiclePushBuffer.clear();
    }

    void multiprocessor::generateVehicle(Vehicle oldVehicle)
    {
        Engine* bufferEngine = oldVehicle.getBufferEngine();
        Vehicle *vehicle = new Vehicle(oldVehicle, oldVehicle.getId() + "_CE", bufferEngine, nullptr);
        Road * belongRoad = oldVehicle.getChangedDrivable()->getBelongRoad();
        // std::cerr << "vehi created" << std::endl;
        vehicle->getControllerInfo()->router.resetAnchorPoints(belongRoad, bufferEngine->getId());
        // std::cerr << "route reset" << std::endl;
        std::lock_guard<std::mutex> guard(lock);
        vehiclePushBuffer.push_back(vehicle);
    }

    void multiprocessor::pushInEngine(int i)
    {
        Vehicle* vehicle = vehiclePushBuffer[i];
        Engine* bufferEngine = vehicle->getBufferEngine();
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