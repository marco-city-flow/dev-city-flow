#include "barrier.h"

namespace CityFlow {
    void Barrier::wait() {
        //std::cerr << "wait start" << std::endl;
        std::unique_lock<std::mutex> lock(m_mutex);
        //std::cerr << "wait lock" << std::endl;

        assert(0u != *currCounter);
        //std::cerr << "asserted done" << std::endl;

        if (!--*currCounter) {
            currCounter += currCounter == counter ? 1 : -1;
            *currCounter = m_threads;
            m_condition.notify_all();
        //std::cerr << "if done" << std::endl;

        } else {
            size_t *currCounter_local = currCounter;
            m_condition.wait(lock, [currCounter_local] { return *currCounter_local == 0; });
        //std::cerr << "else done" << std::endl;

        }
    }
}