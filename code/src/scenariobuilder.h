#ifndef SCENARIOBUILDER_H
#define SCENARIOBUILDER_H

#include "scenario.h"
#include "observablethread.h"
/*
class ScenarioBuilder
{
public:
    ScenarioBuilder();

    QVector<Scenario> generateScenarios(QVector<Scenario> threads);
    QVector<Scenario> generateScenarios(QVector<ObservableThread *> threads);

    void printScenario(QVector<int> scenario);

private:


    bool build(int thread);
    void buildVector(int index);
    Scenario current;
    std::vector<Scenario> currentthreads;
    int nbThreads;
    int scenarioSize;
    std::vector<Scenario> result;
};
*/

#include <condition_variable>

template<typename T> class BufferN {
protected:
    std::vector<T> elements;
    int writePointer{0}, readPointer{0}, nbElements{0}, bufferSize;
    std::mutex mutex;
    std::condition_variable waitProd, waitConso;
    bool finished{false};

public:

    BufferN(unsigned int size) : elements(size), bufferSize(size) {
    }

    virtual ~BufferN() = default;

    int getNbElements() {
        std::unique_lock<std::mutex> lk(mutex);
        return nbElements;
    }

    virtual void put(T item) {
        std::unique_lock<std::mutex> lk(mutex);
        while (nbElements == bufferSize) {
            waitProd.wait(lk);
        }
        elements[writePointer] = item;
        writePointer = (writePointer + 1)
                       % bufferSize;
        nbElements ++;
        waitConso.notify_one();
    }

    virtual T get() {
        T item;
        std::unique_lock<std::mutex> lk(mutex);
        while ((nbElements == 0) && (!finished)) {
            waitConso.wait(lk);
        }
        if (finished) {
            return {};
        }
        item = elements[readPointer];
        readPointer = (readPointer + 1)
                      % bufferSize;
        nbElements --;
        waitProd.notify_one();
        return item;
    }

    virtual void finish() {
        std::unique_lock<std::mutex> lk(mutex);
        finished = true;
        waitConso.notify_one();
    }
};

typedef BufferN<Scenario> Buffer;

class ScenarioBranchBuilder
{
public:
    ScenarioBranchBuilder();

    std::vector<Scenario> generateScenarios(const std::vector<ScenarioGraphNode *> &threads, int depth);
    std::vector<Scenario> generateScenarios(const std::vector<std::unique_ptr<ObservableThread> > &threads, int depth);

    void printScenario(std::vector<int> scenario);

private:


    bool build(int thread, int nextPoint);
    void buildVector(int index);
    Scenario current;
    std::vector<ScenarioGraphNode*> currentthreads;
    int nbThreads{0};
    int scenarioSize{0};
    std::vector<Scenario> result;
};


class ScenarioBranchBuilderBuffer
{
public:
    ScenarioBranchBuilderBuffer(size_t step = 1);

    void generateScenarios(const std::vector<ScenarioGraphNode *> &threads, int depth);
    void generateScenarios(const std::vector<std::unique_ptr<ObservableThread> > &threads, int depth);

    void printScenario(std::vector<int> scenario);

    bool isFinished();

    Buffer *buffer{nullptr};

private:


    bool build(int thread, int nextPoint);
    void buildVector(int index);
    Scenario current;
    std::vector<ScenarioGraphNode*> currentthreads;
    int nbThreads{0};
    int scenarioSize{0};
    std::vector<Scenario> result;
    bool running{true};
    size_t step{1};
    size_t nextIndex{0};
    size_t currentIndex{0};
};




class ScenarioBuilderInterface
{
public:

    virtual ~ScenarioBuilderInterface() = default;
    ///
    /// \brief Initialize the builder
    /// \param threads A vector of observable threads
    /// \param depth The depth of scenarios to generate
    ///
    /// This method shall be called only once, and before any call to getNext().
    ///
    virtual void init(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth) = 0;
    ///
    /// \brief getNext
    /// \return The next scenario. If the scenario is empty, it means there is no more scenario
    ///
    virtual Scenario getNext() = 0;

    ///
    /// \brief getMaxScenariosNb
    /// \return The maximum number of scenarios that can be generated
    ///
    virtual size_t getMaxScenariosNb() = 0;

    virtual size_t getRemainingScenariosNb() = 0;
};

class BruteforceScenarioBuilderIter : public ScenarioBuilderInterface
{
public:

    Scenario getNext() override;
    size_t getMaxScenariosNb() override;
    size_t getRemainingScenariosNb() override;
protected:
    std::vector<Scenario> scenarios;
    size_t currentIndex{0};
};

class UnoptimizedScenarioBuilderIter : public BruteforceScenarioBuilderIter
{
public:
    void init(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth) override;

private:
    ScenarioBranchBuilder builder;

};


class PredefinedScenarioBuilderIter : public BruteforceScenarioBuilderIter
{
public:
    void init(const std::vector<std::unique_ptr<ObservableThread> >& /*threads*/, int /*depth*/) override {}
    void setScenarios(std::vector<Scenario> scenarios);

};


class ScenarioBuilderIter
{
public:
    ScenarioBuilderIter();

    void init(std::vector<ScenarioGraph *> graphs, int depth);
    bool hasNext();
    Scenario getNext();

    std::vector<Scenario> generateScenarios(std::vector<ScenarioGraphNode *> threads, int depth);
    std::vector<Scenario> generateScenarios(const std::vector<std::unique_ptr<ObservableThread> >& threads, int depth);

    void printScenario(std::vector<int> scenario);

private:

    std::vector<ScenarioGraph *> graphs;
    int depth;
    std::vector<ScenarioGraphNode*> currentNodes;
    std::vector<int> currentChildIndex;
    int currentThreadIndex;

    bool build(int thread, int nextPoint);
    void buildVector(int index);
    Scenario current;
    std::vector<ScenarioGraphNode*> currentthreads;
    int nbThreads;
    int scenarioSize;
    std::vector<Scenario> result;
};



class ScenarioBranchBuilderIter
{
public:
    ScenarioBranchBuilderIter() = default;

    void initScenarios(const std::vector<ScenarioGraphNode *> &threads, int depth);
    void initScenarios(const std::vector<std::unique_ptr<ObservableThread> > &threads, int depth);

    void printScenario(std::vector<int> scenario);


    Scenario getNext();

private:


    bool build(int thread, int nextPoint);
    bool buildVector();
    Scenario current;
    std::vector<ScenarioGraphNode*> currentthreads;
    size_t nbThreads{0};
    size_t scenarioSize{0};
    std::vector<Scenario> result;

    std::vector<size_t> int_i;
    std::vector<size_t> int_j;
    std::vector<size_t> int_index;
    std::vector<bool> int_atLeastOneNew;
    std::vector<ScenarioGraphNode*> int_lastBranch;
    size_t currentIndex{0};
    size_t nextCurrentIndex{0};
    Scenario currentReady;
};


class FlowScenarioBuilderIter : public ScenarioBuilderInterface
{
public:

    void init(const std::vector<std::unique_ptr<ObservableThread> > &threads, int depth) override;
    Scenario getNext() override;
    size_t getMaxScenariosNb() override;
    size_t getRemainingScenariosNb() override;
protected:

    ScenarioBranchBuilderIter builder;

};



class ScenarioBuilderBuffer : public ScenarioBuilderInterface
{
public:

    ///
    /// \brief ScenarioBuilderBuffer
    /// \param step interval between two scenarios to be played
    ///
    /// If step is different than 1, then a scenario will be generated every step
    /// only. It allows to arbitrarily play only a subset of all scenarios.
    /// Mainly useful during debugging.
    ///
    ScenarioBuilderBuffer(size_t step = 1) : buffer(10), builder(step) {}

    ~ScenarioBuilderBuffer() override {
        th->join();
    }

    void init(const std::vector<std::unique_ptr<ObservableThread> > &threads, int depth) override;
    Scenario getNext() override;
    size_t getMaxScenariosNb() override;
    size_t getRemainingScenariosNb() override;
    bool isFinished();
protected:

    ScenarioBranchBuilderBuffer builder;

    Buffer buffer;

    std::unique_ptr<std::thread> th;

};


#endif // SCENARIOBUILDER_H
