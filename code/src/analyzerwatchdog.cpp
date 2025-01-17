

#include "analyzerwatchdog.h"
#include "pcoconcurrencyanalyzer.h"

AnalyzerWatchDog::~AnalyzerWatchDog()
{
    // Deregister the watchdog before deletion
    PcoManager::getInstance()->setWatchDog(nullptr);
    mutex.lock();
    finished = true;
    var.notify_one();
    mutex.unlock();
    if (m_thread != nullptr) {
        m_thread->join();
    }
}


void AnalyzerWatchDog::setConcurrencyAnalyzer(std::shared_ptr<PcoConcurrencyAnalyzer> analyzer)
{
    this->analyzer = analyzer;
}

void AnalyzerWatchDog::trigger(int nbBlocked) {
    std::unique_lock<std::mutex> lock(mutex);
    q.push(nbBlocked);
    qA.push(analyzer);
    currentAnalyzer = analyzer;
    var.notify_one();
//        std::cout << "Detected threads that are all blocked" << std::endl;
}

void AnalyzerWatchDog::function()
{
    while (true) {
        int n;
        std::shared_ptr<PcoConcurrencyAnalyzer> a;

        {
            // Let's protect this {} with the mutex
            std::unique_lock<std::mutex> lock(mutex);
            while ((q.empty()) && (!finished)) {
                var.wait(lock);
            }
            if (finished) {
                return;
            }
            n = q.front();
            a = qA.front();
            //std::cout << "N : " << n << "  ";
            q.pop();
            qA.pop();
            // End of protection by the mutex
        }
        a->checkedBlocked(n);
    }
}
void AnalyzerWatchDog::run() {
    m_thread = std::make_unique<std::thread>(&AnalyzerWatchDog::function, this);
}

void AnalyzerWatchDog::terminate() {
    std::unique_lock<std::mutex> lock(mutex);
    finished = true;
    var.notify_one();
}
