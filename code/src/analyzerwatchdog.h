#ifndef ANALYZERWATCHDOG_H
#define ANALYZERWATCHDOG_H

#include <queue>
#include <mutex>
#include <condition_variable>

#include <pcosynchro/pcomanager.h>

class PcoConcurrencyAnalyzer;


class AnalyzerWatchDog : public PcoWatchDog
{
public:
    ~AnalyzerWatchDog() override;

    void run();

    void terminate();


    void setConcurrencyAnalyzer(std::shared_ptr<PcoConcurrencyAnalyzer> analyzer);



private:

    void trigger(int nbBlocked) override;

    std::shared_ptr<PcoConcurrencyAnalyzer> analyzer;

    void function();
    bool finished{false};

    std::mutex mutex;

    std::condition_variable var;

    std::queue<int> q;

    std::queue<std::shared_ptr<PcoConcurrencyAnalyzer> > qA;


    std::shared_ptr<PcoConcurrencyAnalyzer> currentAnalyzer;

    std::unique_ptr<std::thread> m_thread;

};
#endif // ANALYZERWATCHDOG_H
