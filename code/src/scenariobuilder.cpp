#include "scenariobuilder.h"

#include <iostream>



void UnoptimizedScenarioBuilderIter::init(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    scenarios = builder.generateScenarios(threads, depth);
}

size_t BruteforceScenarioBuilderIter::getMaxScenariosNb()
{
    return scenarios.size();
}

size_t BruteforceScenarioBuilderIter::getRemainingScenariosNb()
{
    return scenarios.size() - currentIndex;
}

Scenario BruteforceScenarioBuilderIter::getNext()
{
    if (currentIndex < scenarios.size()) {
        currentIndex ++;
        return scenarios[currentIndex - 1];
    }
    return Scenario();
}

void PredefinedScenarioBuilderIter::setScenarios(std::vector<Scenario> scenarios)
{
    this->scenarios = std::move(scenarios);
}



ScenarioBranchBuilderBuffer::ScenarioBranchBuilderBuffer(size_t step): step(step) {};


bool ScenarioBranchBuilderBuffer::build(int thread, int nextPoint) {
    if (currentthreads[thread]->next.empty())
        return false;
    auto n = currentthreads[thread]->next[nextPoint];
    current.push_back(ScenarioPoint{n->thread, n->number});
    currentthreads[thread] = currentthreads[thread]->next[nextPoint];
    return true;
}

void ScenarioBranchBuilderBuffer::buildVector(int index) {
    bool atLeastOneNew = false;
    for(int i=0;i<nbThreads;i++) {
        for (size_t j = 0; j < currentthreads[i]->next.size(); j++) {
            auto lastBranch = currentthreads[i];
            if (build(i,j)) {
                atLeastOneNew = true;
                if (index == scenarioSize - 1) {
                    if (currentIndex == nextIndex) {
                        buffer->put(current);
                        nextIndex = nextIndex + step;
                        std::cout << ".";
                    }
                    currentIndex++;
                }
                else
                    buildVector(index + 1);
                current.pop_back();
                currentthreads[i] = lastBranch;
            }
        }
    }
    if (!atLeastOneNew) {
        if (currentIndex == nextIndex) {
            buffer->put(current);
            nextIndex = nextIndex + step;
            std::cout << ".";
        }
        currentIndex++;
    }
}


bool ScenarioBranchBuilderBuffer::isFinished()
{
    return !running;
}

void ScenarioBranchBuilderBuffer::generateScenarios(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    running = true;
    std::vector<ScenarioGraphNode*> threadsScenario;
    threadsScenario.reserve(threads.size());
    for(const auto& t: threads)
        threadsScenario.push_back(t->getScenarioGraph()->getFirstNode());
    generateScenarios(threadsScenario, depth);
}

void ScenarioBranchBuilderBuffer::generateScenarios(const std::vector<ScenarioGraphNode*>& threads, int depth)
{
    std::vector<int> choice;
    currentthreads = threads;
    current.clear();
    nbThreads = threads.size();
    scenarioSize = depth;

    buildVector(0);
    running = false;
    buffer->finish();
}










ScenarioBranchBuilder::ScenarioBranchBuilder() = default;

bool ScenarioBranchBuilder::build(int thread, int nextPoint) {
    if (currentthreads[thread]->next.empty())
        return false;
    auto n = currentthreads[thread]->next[nextPoint];
    current.push_back(ScenarioPoint{n->thread, n->number});
    currentthreads[thread] = currentthreads[thread]->next[nextPoint];
    return true;
}

void ScenarioBranchBuilder::buildVector(int index) {
    bool atLeastOneNew = false;
    for(int i=0;i<nbThreads;i++) {
        for (size_t j = 0; j < currentthreads[i]->next.size(); j++) {
            auto lastBranch = currentthreads[i];
            if (build(i,j)) {
                atLeastOneNew = true;
                if (index == scenarioSize - 1) {
                    result.push_back(current);
                }
                else
                    buildVector(index + 1);
                current.pop_back();
                currentthreads[i] = lastBranch;
            }
        }
    }
    if (!atLeastOneNew)
        result.push_back(current);
}



std::vector<Scenario> ScenarioBranchBuilder::generateScenarios(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    std::vector<ScenarioGraphNode*> threadsScenario;
    threadsScenario.reserve(threads.size());
    for(const auto& t: threads)
        threadsScenario.push_back(t->getScenarioGraph()->getFirstNode());
    return generateScenarios(threadsScenario, depth);
}

std::vector<Scenario> ScenarioBranchBuilder::generateScenarios(const std::vector<ScenarioGraphNode*>& threads, int depth)
{
    std::vector<int> choice;
    currentthreads = threads;
    current.clear();
    nbThreads = threads.size();
    scenarioSize = depth;

    buildVector(0);

    return result;
}



void FlowScenarioBuilderIter::init(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    builder.initScenarios(threads, depth);
}

Scenario FlowScenarioBuilderIter::getNext()
{
    return builder.getNext();

}

size_t FlowScenarioBuilderIter::getMaxScenariosNb()
{
    return 0;

}

size_t FlowScenarioBuilderIter::getRemainingScenariosNb()
{
    return 0;
}






































bool ScenarioBranchBuilderIter::build(int thread, int nextPoint) {
    if (currentthreads[thread]->next.empty())
        return false;
    auto n = currentthreads[thread]->next[nextPoint];
    current.push_back(ScenarioPoint{n->thread, n->number});
    currentthreads[thread] = currentthreads[thread]->next[nextPoint];
    return true;
}

bool ScenarioBranchBuilderIter::buildVector() {
    bool result = false;
    while (true) {
        std::cout << currentIndex << ", " << int_i[currentIndex] << ", " << int_j[currentIndex] << std::endl;
        int_lastBranch[currentIndex] = currentthreads[int_i[currentIndex]];
        if (build(int_i[currentIndex],int_j[currentIndex])) {
            int_atLeastOneNew[currentIndex] = true;
            if (currentIndex == scenarioSize - 1) {
    //            result.push_back(current);
                currentReady = current;
                nextCurrentIndex = currentIndex;
            //    result = true;
            }
            else {
                currentIndex += 1;
                // continue to next index
                continue;
            }
            current.pop_back();
            currentthreads[int_i[currentIndex]] = int_lastBranch[currentIndex];
        }

        // If not built, then we need to test a new option
        // If built, also test a new option

        bool ready = false;
        while (!ready) {

            result = true;
            int_j[currentIndex] ++;
            if (int_j[currentIndex] < currentthreads[int_i[currentIndex]]->next.size()) {
                if (currentReady.empty()) {
                    break;
                }
                return true;
            }
            // End of J for loop
            int_j[currentIndex] = 0;
            int_i[currentIndex] ++;
            if (int_i[currentIndex] < nbThreads) {
                // There is still an I to try
                if (currentReady.empty()) {
                    break;
                }
                return true;
            }

            // end if I for loop
            int_i[currentIndex] = 0;
//            if (!int_atLeastOneNew[currentIndex]) {
//                currentReady = current;
                //result = true;
                //                    result.push_back(current);
//            }
            if (currentIndex == 0) {
                // that's the end
            }
            // Go back
            currentIndex --;
        }

        if (!currentReady.empty()) {
            return true;
        }
    }

    return result;
}


/*
 *
bool ScenarioBranchBuilderIter::buildVector() {
    bool result = false;
    std::cout << currentIndex << ", " << int_i[currentIndex] << ", " << int_j[currentIndex] << std::endl;
    while (true) {
        int_lastBranch[currentIndex] = currentthreads[int_i[currentIndex]];
        if (build(int_i[currentIndex],int_j[currentIndex])) {
            int_atLeastOneNew[currentIndex] = true;
            if (currentIndex == scenarioSize - 1) {
    //            result.push_back(current);
                currentReady = current;
                nextCurrentIndex = currentIndex;
            //    result = true;
            }
            else {
                currentIndex += 1;
                if (buildVector()) {
                    return true;
                }
                currentIndex -= 1;
            }
            current.pop_back();
            currentthreads[int_i[currentIndex]] = int_lastBranch[currentIndex];
        }
        result = true;
        int_j[currentIndex] ++;
        if (int_j[currentIndex] >= currentthreads[int_i[currentIndex]]->next.size()) {
            // End of J for loop
            int_j[currentIndex] = 0;
            int_i[currentIndex] ++;
            if (int_i[currentIndex] >= nbThreads) {
                // end if I for loop
                int_i[currentIndex] = 0;
                result = false;
                if (!int_atLeastOneNew[currentIndex]) {
                    currentReady = current;
                    //result = true;
//                    result.push_back(current);
                }
                if (currentIndex == 0) {
                    // that's the end
                }
            }
            else if (currentthreads[int_i[currentIndex]]->next.size() ==0) {
                result = false;
            }
            else if (!currentReady.empty()){
                //result = true;
            }
        }
        if (!currentReady.empty()) {
            // We have a scenario, do not continue iteration for now
            return result;
        }
    }

    return result;
}
*/

void ScenarioBranchBuilderIter::initScenarios(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    std::vector<ScenarioGraphNode*> threadsScenario;
    threadsScenario.reserve(threads.size());
    for(const auto& t: threads)
        threadsScenario.push_back(t->getScenarioGraph()->getFirstNode());
    initScenarios(threadsScenario, depth);
}

void ScenarioBranchBuilderIter::initScenarios(const std::vector<ScenarioGraphNode*>& threads, int depth)
{
    std::vector<int> choice;
    currentthreads = threads;
    current.clear();
    nbThreads = threads.size();
    scenarioSize = depth;

    int_i.resize(scenarioSize, 0);
    int_j.resize(scenarioSize, 0);
    int_index.resize(scenarioSize, 0);
    int_atLeastOneNew.resize(scenarioSize, false);
    int_lastBranch.resize(scenarioSize, nullptr);

    nextCurrentIndex = 0;
    currentIndex = 0;
//    buildVector(0);

}

Scenario ScenarioBranchBuilderIter::getNext()
{
    buildVector();

    auto result = currentReady;
    currentReady = {};

    return result;
}




void ScenarioBuilderBuffer::init(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth)
{
    builder.buffer = &buffer;
    auto *b = &builder;
    th = std::make_unique<std::thread>([b,&threads,depth]{b->generateScenarios(threads, depth);});
}

Scenario ScenarioBuilderBuffer::getNext()
{
    if ((buffer.getNbElements() == 0) && builder.isFinished()) {
        return {};
    }
    return buffer.get();

}

size_t ScenarioBuilderBuffer::getMaxScenariosNb()
{
    return 0;

}

size_t ScenarioBuilderBuffer::getRemainingScenariosNb()
{
    return 0;
}




