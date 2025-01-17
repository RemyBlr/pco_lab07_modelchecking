#include "pcomodel.h"


ScenarioBuilderInterface* PcoModel::getScenarioBuilder() {
    return scenarioBuilder.get();
}

const std::vector<std::unique_ptr<ObservableThread> >& PcoModel::getThreads() {
    return threads;
}
