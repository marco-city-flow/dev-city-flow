#ifndef CITYFLOW_MULTIPROCESSOR_H
#define CITYFLOW_MULTIPROCESSOR_H

#include "flow/flow.h"
#include "roadnet/roadnet.h"
#include "engine/archive.h"
#include "engine/engine.h"
#include "utility/barrier.h"
#include "multiprocessor/ThreadPool.h"

#include <mutex>
#include <thread>
#include <set>
#include <fstream>
#include <string>

namespace CityFlow {
    class multiprocessor{
        friend class Engine;
        friend class RoadNet;

        private:
        RoadNet roadnet;
        std::mutex lock;
        // void initEngineRoad();
        // std::map<Engine *, std::vector<std::pair<Vehicle *, double>>> enginePushBuffer;

        public:
        multiprocessor(const std::string &configFile);

        bool loadFromConfig(std::string);
        void nextStepPro();
        void engineNext(int);
        void pushVehicle(Vehicle *);
        void exchangeVehicle();
        void generateVehicle(Vehicle);
        void updateHistory(int);
        void initEngines(int);
        void pushInEngine(int);

        static std::vector<Engine *> engines;
        std::vector<Vehicle *> vehiclePushBuffer;
    };
}

#endif