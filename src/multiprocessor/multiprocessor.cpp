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
        // std::string cconfigFile = "/home/zhj/Desktop/CityFlow/build/10_10_m/config_10_10.json";
        loadFromConfig(configFile);

        // std::cout << "end of initengines" << std::endl;

        std::vector<std::thread> threads;
        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            threads.emplace_back(std::thread(&multiprocessor::initEngines,this,i));
        }
        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }
        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            multiprocessor::engines[i]->initFlow();
        }
        for (size_t i = 0; i < multiprocessor::engines.size(); ++i)
        {
            multiprocessor::engines[i]->startThread();
        }
        std::cout << "end of init" << std::endl;
    }

    void multiprocessor::initEngines(int i)
    {
        multiprocessor::engines[i]->initId(i);
        multiprocessor::engines[i]->roadnet.initEnginePointer();
        multiprocessor::engines[i]->roadnet.initRoadPointer(engines);
        multiprocessor::engines[i]->initLaneLinks();
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

    void multiprocessor::engineNext50(int i){
        for (size_t j = 0; j < 50; j++)
        {
            multiprocessor::engines[i]->nextStep();
        }
    }

    void multiprocessor::engineNext(int i){
        multiprocessor::engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro_F(){
        // clock_t start, now;
        // start = clock();
        std::vector<std::thread> threads1;
        for(size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            threads1.emplace_back(std::thread(&multiprocessor::engineNext50,this,i));
        }
        for (size_t i = 0; i < threads1.size(); i++)
        {
            threads1[i].join();
        }
        // now = clock();
        // std::cerr << "50 steps" << now - start << std::endl;

        // start = clock();
        std::vector<std::thread> threads2;
        for (size_t i = 0; i < multiprocessor::engines.size(); i++)
        {
            // threads2.emplace_back(std::thread(&multiprocessor::syncChangedVehicles,this,i));
            for (size_t j = 0; j < (engines[i])->virtualFlows.size(); j++)
            {
                threads2.emplace_back(std::thread(&multiprocessor::calDensity,this,i,j));
            }
        }
        for (size_t i = 0; i < threads2.size(); i++)
        {
            threads2[i].join();
        }
        // now = clock();
        // std::cerr << "sync" << now - start << std::endl;
    }

    void multiprocessor::syncFlow(int i){
        engines[i]->syncFlow(i);
    }

    void multiprocessor::syncChangedVehicles(int i)
    {
        engines[i]->syncChangedVehicles(i);
    }

    void multiprocessor::calDensity(int engineId, int flowId){
        engines[engineId]->virtualFlows[flowId]->calDensity();
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

    /* APIs */
    /* getters */
    size_t multiprocessor::getVehicleCount() const{
        size_t totalCount = 0;
        for (auto engine : engines){
            totalCount += engine->getVehicleCount();
        }
        return totalCount;
    }

    std::vector<std::string> multiprocessor::getVehicles(bool includeWaiting) const {
        std::vector<std::string> ret;
        for (auto engine : engines){
            auto engineVehicles = engine->getVehicles(includeWaiting);
            ret.insert(ret.end(), engineVehicles.begin(), engineVehicles.end());
        }
        return ret;
    }

    std::map<std::string, int> multiprocessor::getLaneVehicleCount() const{
        std::map<std::string, int> ret;
        for (auto engine : engines){
            auto laneVehicleCount = engine->getLaneVehicleCount();
            ret.insert(laneVehicleCount.begin(), laneVehicleCount.end());
        }
        return ret;
    }

    std::map<std::string, int> multiprocessor::getLaneWaitingVehicleCount() const{
        std::map<std::string, int> ret;
        for (auto engine : engines){
            auto laneVehicleCount = engine->getLaneWaitingVehicleCount();
            ret.insert(laneVehicleCount.begin(), laneVehicleCount.end());
        }
        return ret;
    }

    std::map<std::string, std::vector<std::string>> multiprocessor::getLaneVehicles(){
        std::map<std::string, std::vector<std::string>> ret;
        for (auto engine : engines){
            auto laneVehicles = engine->getLaneVehicles();
            ret.insert(laneVehicles.begin(), laneVehicles.end());
        }
        return ret;
    }

    std::map<std::string, double> multiprocessor::getVehicleSpeed() const{
        std::map<std::string, double> ret;
        for (auto engine : engines){
            auto vehicleSpeed = engine->getVehicleSpeed();
            ret.insert(vehicleSpeed.begin(), vehicleSpeed.end());
        }
        return ret;
    }

    std::map<std::string, double> multiprocessor::getVehicleDistance() const{
        std::map<std::string, double> ret;
        for (auto engine : engines){
            auto vehicleDistance = engine->getVehicleDistance();
            ret.insert(vehicleDistance.begin(), vehicleDistance.end());
        }
        return ret;
    }

    std::map<std::string, std::string> multiprocessor::getVehicleInfo(const std::string &id) const{
        size_t tested = 0;
        std::map<std::string, std::string> ret;
        for (auto engine : engines){
            try{
                ret = engine -> getVehicleInfo(id);
                break;  /* break the loop if no exception is thrown, i.e. vehicle #id found */
            }
            catch(std::runtime_error& rt_err){
                tested++;
            }
        }

        if(tested == engines.size()){
            throw std::runtime_error("Vehicle '" + id + "' not found");
        }

        return ret;
    }

    std::string multiprocessor::getLeader(const std::string &vehicleId) const {
        for (auto engine : engines){
            if (engine -> getLeader(vehicleId) != ""){
                return engine -> getLeader(vehicleId);
            }
        }
        return "";
    }

    double multiprocessor::getCurrentTime() const {
        return engines[0]->step * engines[0]->interval;
    }

    double multiprocessor::getAverageTravelTime() const {
        double totalTime = 0.0;
        int totalN = 0;
        for (auto engine : engines){
            totalTime += engine->getAverageTravelTime() * engine->finishedVehicleCnt;
            totalN += engine->finishedVehicleCnt;
        }
        return totalN == 0 ? 0 : totalTime / totalN;
    }

    /* setters */
    void multiprocessor::setTrafficLightPhase(const std::string &id, int phaseIndex){
        size_t tested = 0;
        for (auto engine : engines){
            try{
                engine -> setTrafficLightPhase(id, phaseIndex);
            }
            catch(std::runtime_error& rt_err){
                tested++;
            }
        }

        if(tested == engines.size()){
            throw std::runtime_error("Intersection '" + id + "' not found");
        }
    }

    void multiprocessor::setReplayLogFile(const std::string &logFile){
        for (auto engine : engines){
            engine -> setReplayLogFile(logFile);
        }
    }

    void multiprocessor::setVehicleSpeed(const std::string &id, double speed){
        size_t tested = 0;
        for (auto engine : engines){
            try{
                engine -> setVehicleSpeed(id, speed);
            }
            catch(std::runtime_error& rt_err){
                tested++;
            }
        }

        if(tested == engines.size()){
            throw std::runtime_error("Vehicle '" + id + "' not found");
        }
    }

    void multiprocessor::setSaveReplay(bool open){
        for (auto engine : engines){
            engine -> setSaveReplay(open);
        }
    }

    /* others */
    // TODO: implement engine/archive.cpp
    void multiprocessor::reset(bool resetRnd) {
        for (auto engine : engines){
            engine -> reset(resetRnd);
        }
    }

    void multiprocessor::loadFromFile(const char *fileName) {
        // Archive archive(*this, fileName);
        // archive.resume(*this);
    }

    bool multiprocessor::setRoute(const std::string &vehicle_id, const std::vector<std::string> &anchor_id){
        for (auto engine : engines){
            if(engine -> setRoute(vehicle_id, anchor_id))
                return true;
        }
        return false;
    }
}