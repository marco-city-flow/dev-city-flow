#include "engine/engine.h"
#include "engine/archive.h"

#include "pybind11/pybind11.h"
#include "pybind11/stl.h"

namespace py = pybind11;
using namespace py::literals;

PYBIND11_MODULE(cityflow, m) {
    py::class_<CityFlow::multiprocessor>(m,"multiprocessor")
        .def(py::init<const std::string&>(),
            "config_file"_a
        )
        .def("next_step_pro",&CityFlow::multiprocessor::nextStepPro)
        .def("get_current_time", &CityFlow::multiprocessor::getCurrentTime)
        .def("get_vehicle_distance", &CityFlow::multiprocessor::getVehicleDistance)
        .def("get_vehicle_speed", &CityFlow::multiprocessor::getVehicleSpeed)
        .def("get_lane_waiting_vehicle_count", &CityFlow::multiprocessor::getLaneWaitingVehicleCount)
        .def("get_lane_vehicle_count", &CityFlow::multiprocessor::getLaneVehicleCount)
        .def("get_vehicle_speed", &CityFlow::multiprocessor::getVehicleSpeed)
        .def("get_lane_vehicles", &CityFlow::multiprocessor::getLaneVehicles)
        .def("set_tl_phase", &CityFlow::multiprocessor::setTrafficLightPhase, "intersection_id"_a, "phase_id"_a)
        .def("next_step_pro_F",&CityFlow::multiprocessor::nextStepPro_F, "num"_a);
    py::class_<CityFlow::Engine>(m, "Engine")
        .def(py::init<const std::string&, int, CityFlow::multiprocessor*>(),
            "config_file"_a,
            "thread_num"_a=1,
            "multiprocessor"_a = nullptr
        )
        .def("next_step", &CityFlow::Engine::nextStep)
        .def("get_vehicle_count", &CityFlow::Engine::getVehicleCount)
        .def("get_vehicles", &CityFlow::Engine::getVehicles, "include_waiting"_a=false)
        .def("get_lane_vehicle_count", &CityFlow::Engine::getLaneVehicleCount)
        .def("get_lane_waiting_vehicle_count", &CityFlow::Engine::getLaneWaitingVehicleCount)
        .def("get_lane_vehicles", &CityFlow::Engine::getLaneVehicles)
        .def("get_vehicle_speed", &CityFlow::Engine::getVehicleSpeed)
        .def("get_vehicle_info", &CityFlow::Engine::getVehicleInfo, "vehicle_id"_a)
        .def("get_vehicle_distance", &CityFlow::Engine::getVehicleDistance)
        .def("get_leader", &CityFlow::Engine::getLeader, "vehicle_id"_a)
        .def("get_current_time", &CityFlow::Engine::getCurrentTime)
        .def("get_average_travel_time", &CityFlow::Engine::getAverageTravelTime)
        .def("set_tl_phase", &CityFlow::Engine::setTrafficLightPhase, "intersection_id"_a, "phase_id"_a)
        .def("set_vehicle_speed", &CityFlow::Engine::setVehicleSpeed, "vehicle_id"_a, "speed"_a)
        .def("set_replay_file", &CityFlow::Engine::setReplayLogFile, "replay_file"_a)
        .def("set_random_seed", &CityFlow::Engine::setRandomSeed, "seed"_a)
        .def("set_save_replay", &CityFlow::Engine::setSaveReplay, "open"_a)
        .def("push_vehicle", (void (CityFlow::Engine::*)(const std::map<std::string, double>&, const std::vector<std::string>&)) &CityFlow::Engine::pushVehicle)
        .def("reset", &CityFlow::Engine::reset, "seed"_a=false)
        .def("load", &CityFlow::Engine::load, "archive"_a)
        .def("snapshot", &CityFlow::Engine::snapshot)
        .def("load_from_file", &CityFlow::Engine::loadFromFile, "path"_a)
        .def("set_vehicle_route", &CityFlow::Engine::setRoute, "vehicle_id"_a, "route"_a);

    py::class_<CityFlow::Archive>(m, "Archive")
        .def(py::init<const CityFlow::Engine&>())
        .def("dump", &CityFlow::Archive::dump, "path"_a);

#ifdef VERSION
    m.attr("__version__") = VERSION;
#else
    m.attr("__version__") = "dev";
#endif
}
