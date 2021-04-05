#include "flow/flow.h"
#include "engine/engine.h"


namespace CityFlow {
    void Flow::nextStep(double timeInterval) {
        if (!valid) return;
        if (endTime != -1 && currentTime > endTime) return;
        if (currentTime >= startTime) {
            while (nowTime >= interval) {
                Vehicle* vehicle = new Vehicle(vehicleTemplate, id + "_" + std::to_string(cnt++), engine, this);
                int priority = vehicle->getPriority();
                while (engine->checkPriority(priority)) priority = engine->rnd();
                vehicle->setPriority(priority);
                engine->pushVehicle(vehicle, false);
                vehicle->getFirstRoad()->addPlanRouteVehicle(vehicle);
                nowTime -= interval;
            }
            nowTime += timeInterval;
        }
        currentTime += timeInterval;
    }

    std::string Flow::getId() const {
        return id;
    }

    void Flow::setEndTime(int engineId)
    {
        endTime = lane->getDensity();
        if (endTime != 0)
        {
            std::cerr << "endTime:" << endTime << std::endl;
            lane->resetDensity();
            vehicleTemplate = tempInfo;
            resetRoute(engineId);
        }
    }

    void Flow::reset() {
        nowTime = interval;
        currentTime = 0;
        cnt = 0;
    }

    void Flow::resetRoute(int engineId)
    {
        std::vector<Road *> roads;
        roads.push_back(lane->getBelongRoad());
        roads.push_back(vehicleTemplate.route->getRoute().back());//tochange pointer
        vehicleTemplate.route = std::make_shared<const Route>(roads);
    }
}