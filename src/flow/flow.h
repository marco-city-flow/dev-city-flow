#ifndef CITYFLOW_FLOW_H
#define CITYFLOW_FLOW_H

#include <iostream>

#include "vehicle/vehicle.h"
#include "flow/route.h"
#include "multiprocessor/multiprocessor.h"
namespace CityFlow {
    class Engine;

    struct VehicleInfo;

    class Flow {
        friend class Archive;
        friend class multiprocessor;
    private:
        VehicleInfo vehicleTemplate;
        Vehicle* templateVehicle;
        VehicleInfo tempInfo;
        std::shared_ptr<const Route> route;
        double interval = 2;
        double nowTime = 0;
        double currentTime = 0;
        int startTime = 0;
        int endTime = -1;
        int cnt = 0;
        bool isVirtual = false;
        Engine *engine;
        std::string id;
        bool valid = true;
        Lane *lane;
        Road *endRoad = nullptr;
        std::vector<Vehicle> vehicleBuffer;
        std::vector<Flow*> flowBuffer;
        Vehicle *waitForPushVehicle = nullptr;
        int receiveVehicle = 0;

    public:
        Flow(Lane *lane, Engine *engine) : vehicleTemplate(), endTime(0), engine(engine), lane(lane){
            isVirtual = true;
        };

        Flow(const VehicleInfo &vehicleTemplate, double timeInterval,
            Engine *engine, int startTime, int endTime, const std::string &id)
            : vehicleTemplate(vehicleTemplate), interval(timeInterval),
              startTime(startTime), endTime(endTime), engine(engine), id(id) {
            assert(timeInterval >= 1 || (startTime == endTime));
            nowTime = interval;
        }

        void nextStep(double timeInterval);

        std::string getId() const;

        void setEndTime(int engineId);

        bool getVirtual() const { return isVirtual; }

        bool isValid() const { return this->valid; }

        VehicleInfo getTemplate() const { return vehicleTemplate; }

        void setValid(const bool valid) {
            if (this->valid && !valid) {
                std::cerr << "[warning] Invalid route '" << id << "'. Omitted by default." << std::endl;
                std::cerr << this->endRoad->getId() << std::endl;
            }
            this->valid = valid;
        }

        void setTemplate(Vehicle vehicle) { tempInfo = vehicle.getTemplate(); endRoad = vehicle.getControllerInfo()->endRoad; std::cerr << "endRoad: " << endRoad << std::endl; }

        void reset();

        void resetRoute(int engineId);

        void addToBuffer(Vehicle vehicle, Flow* flow);

        Road* getEndRoad() const { return endRoad; }

        void calDensity();

        void setId(std::string id) { this->id = id; };

        void pushToEngine();
    };
}

#endif //CITYFLOW_FLOW_H
