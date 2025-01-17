#include <iostream>

#include "observablethread.h"

#include "pcoconcurrencyanalyzer.h"


void startSection(int id) {
    ObservableThread::getCurrentObservable()->obStartSection(id);
}

void endSection() {
    ObservableThread::getCurrentObservable()->obEndSection();
}

void endScenario() {
    ObservableThread::getCurrentObservable()->obEndScenario();
}


std::vector<ObservableThread*> ObservableThread::allThreads;
std::mutex ObservableThread::mutex;

bool ObservableThread::verbose{false};

void ObservableThread::setConcurrencyAnalyzer(PcoConcurrencyAnalyzer *analyzer)
{
    this->analyzer = analyzer;
}

void ObservableThread::obStartSection(int section)
{
    if (verbose) {
        std::cout << "Thread " << this->id << " in  startSection ( " << section << " ) " << std::endl;
    }
    analyzer->startSection(this,section);
    if (verbose) {
        std::cout << "Thread " << this->id << " out startSection ( " << section << " ) " << std::endl;
    }
}

void ObservableThread::obEndSection()
{
    if (verbose) {
        std::cout << "Thread " << this->id << " in  endSection" << std::endl;
    }
    analyzer->endSection(this);
    if (verbose) {
        std::cout << "Thread " << this->id << " out endSection" << std::endl;
    }
}

void ObservableThread::obEndScenario()
{
    if (verbose) {
        std::cout << "Thread " << this->id << " in  endScenario" << std::endl;
    }
    analyzer->endScenario(this);
    if (verbose) {
        std::cout << "Thread " << this->id << " out endScenario" << std::endl;
    }
}
