#include <iostream>

#include "pcomodelchecker.h"

void PcoModelChecker::setModel(PcoModel *model) {
    this->model = model;
}


void PcoModelChecker::run() {

    // First build the model
    model->build();

    // Creation of the watchdog and start of this watchdog
    AnalyzerWatchDog watchDog;
    PcoManager::getInstance()->setWatchDog(&watchDog);
    watchDog.run();

    // Iterate over all the scenarios, using the scenariobuilder iterator
    for (Scenario scenario = model->getScenarioBuilder()->getNext(); !scenario.empty();
         scenario = model->getScenarioBuilder()->getNext()) {

        // To be sure we start from scratch we create a new analyzer
        auto analyzer = std::make_shared<PcoConcurrencyAnalyzer>();

        watchDog.setConcurrencyAnalyzer(analyzer);

        analyzer->setModel(model);

        analyzer->setScenario(scenario, model->getThreads().size());
        // The following lines could be used if the scenario builder has a getRemainingScenariosNb() function,
        // but this is currently not the case
        //            if ((model->getScenarioBuilder()->getRemainingScenariosNb() % 100) == 0) {
        //                std::cout << model->getScenarioBuilder()->getRemainingScenariosNb() << std::endl;
        //            }

        // Allow the model to set things before starting
        model->preRun(scenario);

        // Set the analyzer of all threads
        for (auto & thread : model->getThreads())
            thread->setConcurrencyAnalyzer(analyzer.get());

        // Start the threads
        for (auto & thread : model->getThreads())
            thread->start();

        // And join them
        for (auto & thread : model->getThreads())
            thread->join();

        // Update the ending status map
        endingStatusCounter[analyzer->getEndingStatus()]++;

        // Allow the model to do something at the end of the scenario
        model->postRun(scenario);

        // TODO : Do this depending on a verbosity level
        // printEndingStatus(analyzer->getEndingStatus());
    }

    // Stop the watchdog
    watchDog.terminate();

    // Print statistics about the ending status of each scenario
    printStats();

    // Write the model final report
    model->finalReport();
}

void PcoModelChecker::printEndingStatus(PcoConcurrencyAnalyzer::EndingStatus endingStatus)
{
    switch (endingStatus) {
    case PcoConcurrencyAnalyzer::EndingStatus::Unknown: std::cout << "End: Unknown" << std::endl;break;
    case PcoConcurrencyAnalyzer::EndingStatus::Depth: std::cout << "End: Depth" << std::endl;break;
    case PcoConcurrencyAnalyzer::EndingStatus::Deadlock: std::cout << "End: Deadlock" << std::endl;break;
    case PcoConcurrencyAnalyzer::EndingStatus::EndAllScenario: std::cout << "End: AllScenario" << std::endl;break;
    case PcoConcurrencyAnalyzer::EndingStatus::DeadEnd: std::cout << "End: DeadEnd" << std::endl;break;
    }
}

void PcoModelChecker::printStats()
{
    std::cout << "End : Unknown     : " <<  endingStatusCounter[PcoConcurrencyAnalyzer::EndingStatus::Unknown] << std::endl;
    std::cout << "End : Depth       : " <<  endingStatusCounter[PcoConcurrencyAnalyzer::EndingStatus::Depth] << std::endl;
    std::cout << "End : Deadlock    : " <<  endingStatusCounter[PcoConcurrencyAnalyzer::EndingStatus::Deadlock] << std::endl;
    std::cout << "End : AllScenario : " <<  endingStatusCounter[PcoConcurrencyAnalyzer::EndingStatus::EndAllScenario] << std::endl;
    std::cout << "End : DeadEnd     : " <<  endingStatusCounter[PcoConcurrencyAnalyzer::EndingStatus::DeadEnd] << std::endl;
}
