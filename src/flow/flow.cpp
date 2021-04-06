#include "flow/flow.h"
#include "engine/engine.h"


namespace CityFlow {
    void Flow::nextStep(double timeInterval) {
        if (!valid) return;
        if (endTime != -1 && currentTime > endTime) return;
        if (!isVirtual){
            if (currentTime >= startTime) {
                while (nowTime >= interval && endTime != 0) {
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
        }
        else
        {
            if (currentTime < endTime) {
                Vehicle *vehicle = new Vehicle(*templateVehicle, this);
                waitForPushVehicle = vehicle;

                waitForPushVehicle->setFirstDrivable();
                Lane *lane = (Lane *)(waitForPushVehicle->getChangedDrivable());
                Vehicle * tail = lane->getLastVehicle();
                lane->pushVehicle(waitForPushVehicle);
                waitForPushVehicle->updateLeaderAndGap(tail);
                // std::cerr << "leaderandgap update" << std::endl;

                waitForPushVehicle->update();
                waitForPushVehicle->clearSignal();
            }
        }
        currentTime += timeInterval;
    }

    void Flow::pushToEngine()
    {
        if (waitForPushVehicle != nullptr)
        {
            int priority = waitForPushVehicle->getPriority();
            while (engine->checkPriority(priority)) priority = engine->rnd();
            waitForPushVehicle->setPriority(priority);
            engine->pushVehicle(waitForPushVehicle, false);
            engine->addActiveVehicle();

            waitForPushVehicle = nullptr;
        }
    }

    std::string Flow::getId() const {
        return id;
    }

    void Flow::setEndTime(int engineId)
    {
        // endTime = lane->getDensity();
        // // std::cerr << "setEndTime: " << lane->getBelongRoad()->getId() << std::endl;
        // if (endTime != 0 && endRoad != nullptr)
        // {
        //     std::cerr << "endTime:" << endTime << std::endl;
        //     lane->resetDensity();
        //     currentTime = 0;
        //     vehicleTemplate = tempInfo;
        //     resetRoute(engineId);
        // }
    }

    void Flow::reset() {
        nowTime = interval;
        currentTime = 0;
        cnt = 0;
    }

    // void Flow::resetRoute(int engineId)
    // {
    //     std::vector<Road *> roads;
    //     roads.push_back(lane->getBelongRoad());
    //     // std::cerr << "resetRoutestart: " << roads.back()->getId() << std::endl;
    //     roads.push_back(endRoad->getSameRoadById(engineId));//tochange pointer
    //     // std::cerr << "resetRouteend: " << roads.back()->getId() << std::endl;
    //     vehicleTemplate.route = std::make_shared<const Route>(roads);
    // }

    void Flow::addToBuffer(Vehicle vehicle)
    {
        receiveVehicle++;
        if (vehicleBuffer.size() == 0)
        {
            vehicleBuffer.push_back(vehicle);
        }
    }

    void Flow::calDensity()
    {
        // endTime = vehicleBuffer.size();
        endTime = receiveVehicle;
        if (endTime != 0)
        {
            // std::cerr << "calDensity: " << endTime << std::endl;
            // vehicleTemplate = vehicleBuffer.begin()->getTemplate();
            templateVehicle = new Vehicle(*(vehicleBuffer.begin()), vehicleBuffer.begin()->getId() + "_CE", engine, nullptr);
            Road * belongRoad = vehicleBuffer.begin()->getChangedDrivable()->getBelongRoad();
            templateVehicle->getControllerInfo()->router.resetAnchorPoints(belongRoad, engine->getId());
            templateVehicle->updateRoute();
            // std::cerr << "route update" << std::endl;

            endRoad = vehicleBuffer.begin()->getControllerInfo()->router.getLastRoad();
            vehicleBuffer.clear();
            receiveVehicle = 0;
        }
        currentTime = 0;
    }
}