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
        void nextStepPro_F();
        void engineNext(int);
        void engineNext50(int);
        void pushVehicle(Vehicle *);
        void exchangeVehicle();
        void generateVehicle(Vehicle);
        void updateHistory(int);
        void syncFlow(int);
        void syncChangedVehicles(int i);
        void initEngines(int);
        void pushInEngine(int);
        void calDensity(int, int);


        /*----------------------------------------------------------------*/
        /*|                             A P I s                          |*/
        /*|                      cityflow.cpp L22 - L44                  |*/
        /*----------------------------------------------------------------*/
        /* getters */
        size_t getVehicleCount() const;
        std::vector<std::string> getVehicles(bool includeWaiting = false) const;
        std::map<std::string, int> getLaneVehicleCount() const;
        std::map<std::string, int> getLaneWaitingVehicleCount() const;
        std::map<std::string, std::vector<std::string>> getLaneVehicles();
        std::map<std::string, double> getVehicleSpeed() const;
        std::map<std::string, std::string> getVehicleInfo(const std::string &id) const;
        std::map<std::string, double> getVehicleDistance() const;
        std::string getLeader(const std::string &vehicleId) const;
        double getCurrentTime() const;
        double getAverageTravelTime() const;

        /* setters */
        void setTrafficLightPhase(const std::string &id, int phaseIndex);
        void setReplayLogFile(const std::string &logFile);
        void setSaveReplay(bool open);
        void setVehicleSpeed(const std::string &id, double speed);
        void setRandomSeed(int seed);

        /* others */
        //TODO engine/archive.h
        void reset(bool resetRnd = false);
        void load(const Archive &archive) { archive.resume(*this); }
        Archive snapshot(){ return Archive(*this);}
        void loadFromFile(const char *fileName);
        bool setRoute(const std::string &vehicle_id, const std::vector<std::string> &anchor_id);
        /* how? */void pushVehicle(const std::map<std::string, double> &info, const std::vector<std::string> &roads);//TODO

        /*----------------------------------------------------------------*/

        static std::vector<Engine *> engines;
        std::vector<Vehicle *> vehiclePushBuffer;
    };
}

#endif