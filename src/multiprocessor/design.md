```c++
    void Engine::threadController(std::set<Vehicle *> &vehicles,
                                  std::vector<Road *> &roads,
                                  std::vector<Intersection *> &intersections,
                                  std::vector<Drivable *> &drivables) {
        while (!finished) {
            threadPlanRoute(roads);
            if (laneChange) {
                threadInitSegments(roads);
                threadPlanLaneChange(vehicles);
                threadUpdateLeaderAndGap(drivables);
            }
            threadNotifyCross(intersections);
            threadGetAction(vehicles);
            threadUpdateLocation(drivables);
            threadUpdateAction(vehicles);
            threadUpdateLeaderAndGap(drivables);
        }
    }
```

每个step先计算nextstep再计算threadcontrol

#### threadplanroute

需要修改vehicle里的updateroute，这一段dijkstra后面会修改先不改动

对于multi每个模块存储整个地图，或者在中枢做路径规划（二选一）