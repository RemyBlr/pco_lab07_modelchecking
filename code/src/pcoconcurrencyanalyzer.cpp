
#include <iostream>

#include "pcosynchro/pcomanager.h"

#include "pcoconcurrencyanalyzer.h"



PcoConcurrencyAnalyzer *PcoConcurrencyAnalyzer::currentAnalyzer;


PcoConcurrencyAnalyzer::PcoConcurrencyAnalyzer()
{
    currentAnalyzer = this;
    nbWaiting = 0;
    currentThread = nullptr;
    index = 0;
    aborting = false;
    nbRunningThreads = nbThreads;
    PcoManager::getInstance()->setNormalMode();
}

PcoConcurrencyAnalyzer::~PcoConcurrencyAnalyzer() = default;

void PcoConcurrencyAnalyzer::setScenario(Scenario s, unsigned int nbThreads)
{
    scenario = std::move(s);
    this->nbThreads = nbThreads;
    start();
}


const Scenario& PcoConcurrencyAnalyzer::getScenario() const
{
    return scenario;
}


PcoConcurrencyAnalyzer::EndingStatus PcoConcurrencyAnalyzer::getEndingStatus()
{
    return endingStatus;
}


void PcoConcurrencyAnalyzer::start()
{
    std::lock_guard lock(mutex);
    currentThread = nullptr;
    index = 0;
    aborting = false;
    nbRunningThreads = nbThreads;
    nbWaiting = 0;
    PcoManager::getInstance()->setNormalMode();
}

#define ENDING {nbRunningThreads--;PcoThread::exitThread();}


void PcoConcurrencyAnalyzer::startSection(ObservableThread *thread, int sectionNumber)
{
    std::unique_lock<std::mutex> lock(mutex);
    if (aborting) {
        ENDING;
        return;
    }
    if (currentThread == thread) {
        index ++;
        currentThread = nullptr;
    }
    if (index == scenario.size()) {
        endingStatus = EndingStatus::Depth;
        PcoManager::getInstance()->setFreeMode();
        aborting = true;
        for (int i = 0; i < nbWaiting; i++) {
            waiting.notify_one();
        }
        nbWaiting = 0;
        ENDING;
        return;
    }


    while (nbWaiting > 0) {
        waiting.notify_one();
        nbWaiting --;
    }

    while ((scenario.at(index).number != sectionNumber) || (scenario.at(index).thread != thread)) {

        if ((nbWaiting + PcoManager::getInstance()->nbBlockedThreads() == nbRunningThreads - 1)) {

            endingStatus = EndingStatus::DeadEnd;
            PcoManager::getInstance()->setFreeMode();

            aborting = true;
            for (int i = 0; i < nbWaiting; i++) {
                waiting.notify_one();
            }
            nbWaiting = 0;
            ENDING;
            return;
        }
        nbWaiting ++;
        waiting.wait(lock);
        if (aborting) {
            ENDING;
            return;
        }
    }
    currentThread = thread;

    checkInvariants();
}

void PcoConcurrencyAnalyzer::endSection(ObservableThread *thread)
{
    std::lock_guard lock(mutex);
    if (currentThread == thread) {
        index ++;
        currentThread = nullptr;
        if (index == scenario.size()) {
            //std::cout << Scenario::toString(scenario) << "End of scenario (max depth reached)" << std::endl;
            endingStatus = EndingStatus::Depth;
            aborting = true;
            for (int i = 0; i < nbWaiting; i++) {
                waiting.notify_one();
            }
            nbWaiting = 0;
            ENDING;
            return;
        }
        while (nbWaiting > 0) {
            waiting.notify_one();
            nbWaiting --;
        }
    }
    checkInvariants();
}

void PcoConcurrencyAnalyzer::endScenario(ObservableThread *thread)
{
    std::lock_guard lock(mutex);
    nbRunningThreads --;
    if (!aborting) {
        if (nbRunningThreads == 0) {
            endingStatus = EndingStatus::EndAllScenario;
        }
        else if (currentThread == thread) {
            index ++;
            currentThread = nullptr;
            if (index == scenario.size()) {
                //std::cout << Scenario::toString(scenario) << "End of scenario (max depth reached)" << std::endl;
                endingStatus = EndingStatus::Depth;
                aborting = true;
                for (int i = 0; i < nbWaiting; i++) {
                    waiting.notify_one();
                }
                nbWaiting = 0;
                ENDING;
                return;
            }
            while (nbWaiting > 0) {
                waiting.notify_one();
                nbWaiting --;
            }
        }
    }
    checkInvariants();
}

void PcoConcurrencyAnalyzer::checkedBlocked(int /*nbBlocked*/)
{
    std::lock_guard lock(mutex);

    while (nbWaiting > 0) {
        waiting.notify_one();
        nbWaiting --;
    }

    if ((!aborting) && (endingStatus == EndingStatus::Unknown)) {
        if ((PcoManager::getInstance()->nbBlockedThreads() == nbRunningThreads) && (nbRunningThreads != 0)) {
            // std::cout << "Checker ending" << std::endl;
            endingStatus = EndingStatus::Deadlock;
            PcoManager::getInstance()->setFreeMode();
            aborting = true;
            currentThread = nullptr;
        }
    }
}

bool PcoConcurrencyAnalyzer::aborted()
{
    // To be checked: if the mutex here does not cause a deadlock
    std::lock_guard lock(mutex);
    return aborting;
}

void PcoConcurrencyAnalyzer::setModel(PcoModel *model) {
    this->model = model;
}

void PcoConcurrencyAnalyzer::checkInvariants() {
    if (model != nullptr) {
        if (!model->checkInvariants()) {
            std::cout << "****************************************************" << std::endl;
            std::cout << "Detected an error" << std::endl;
            std::cout << ScenarioPrint::toString(scenario) << std::endl;
            std::cout << "****************************************************" << std::endl;
        }
    }
}
