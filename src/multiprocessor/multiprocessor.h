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

namespace CityFlow {
    class multiprocessor{
        private:
        RoadNet roadnet;//整个地图 用于规划路径 或者查找？
        std::vector<Engine *> engines;
        std::mutex lock;
        // std::map<Engine *, std::vector<std::pair<Vehicle *, double>>> enginePushBuffer;

        public:
        multiprocessor();//读取核心文件并进行切分 这里暂且省略直接用假数据
        void nextStepPro();
        void engineNext(int);
        void pushVehicle(Vehicle *);
        void exchangeVehicle();
    };
}

#endif