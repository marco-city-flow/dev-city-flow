#include "multiprocessor/multiprocessor.h"
#include <unistd.h>
namespace CityFlow{
    multiprocessor::multiprocessor():pool(4)
    {
        Engine* engine = new Engine("./10_10_1/config_10_10.json",10);
        engines.push_back(engine);
        engine = new Engine("./10_10_2/config_10_10.json",10);
        engines.push_back(engine);
        engine = new Engine("./10_10_3/config_10_10.json",10);
        engines.push_back(engine);
        engine = new Engine("./10_10_4/config_10_10.json",10);
        engines.push_back(engine);
        std::cout << "end of init" << std::endl;
    }

    void multiprocessor::engineNext(int i){
        engines[i]->nextStep();
    }

    void multiprocessor::nextStepPro()
    {
        std::vector<std::thread> threads;
        for(size_t i = 0; i < engines.size(); i++)
        {
            threads.emplace_back(std::thread(&multiprocessor::engineNext,this,i));
        }
        for (size_t i = 0; i < threads.size(); i++)
        {
            threads[i].join();
        }
    }
}