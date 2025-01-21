#ifndef MODELTEMPLATE_H
#define MODELTEMPLATE_H

#include <iostream>
#include <memory>

#include "pcomodel.h"
#include "pcoconcurrencyanalyzer.h"
#include "pcosynchro/pcosemaphore.h"

/**
 * @brief Interface abstraite pour un buffer générique
 */
template<typename T>
class AbstractBuffer {
public:
    virtual ~AbstractBuffer() = default;
    virtual void put(T item) = 0;
    virtual T get() = 0;
};

/**
 * @brief Buffer2ConsoSemaphore
 *
 * Code basé sur le corrigé fourni par Monsieur Cuisenaire : ex15.1_alexandre.h
 *
 * - 2 "get()"
 * - 1 "put()"
 */
template<typename T>
class Buffer2ConsoSemaphore : public AbstractBuffer<T> {
protected:
    PcoSemaphore mutex;
    PcoSemaphore waitFull;
    PcoSemaphore waitEmpty;
    T element;

public:
    Buffer2ConsoSemaphore()
        : mutex(1), waitFull(0), waitEmpty(2){}

    ~Buffer2ConsoSemaphore() override = default;

    void put(T item) override {
        mutex.acquire();
        waitEmpty.acquire();
        waitEmpty.acquire();
        mutex.release();

        // Stock item
        element = item;

        // signal element is ready for consumer
        waitFull.release();
        waitFull.release();
    }

    T get() override {
        // wait for element to be ready
        waitFull.acquire();
        // get element
        T item = element;
        // realease empty spot
        waitEmpty.release();
        return item;
    }
};


/**
 * @brief Thread producteur
 *
 * Il y a 3 sections (1, 2, 3) pour avoir plus de permutations.
 * - Section 1 : préparation (fictive)
 * - Section 2 : put(...)
 * - Section 3 : éventuellement un post-traitement
 */
class ProducerThread : public ObservableThread
{
public:
    ProducerThread(std::shared_ptr<AbstractBuffer<int>> buffer, std::string id = "")
        : ObservableThread(std::move(id)),
          buffer(std::move(buffer))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();

        auto firstNode = scenarioGraph->createNode(this, -1);
        auto s1 = scenarioGraph->createNode(this, 1);
        auto s2 = scenarioGraph->createNode(this, 2);
        auto s3 = scenarioGraph->createNode(this, 3);

        firstNode->next.push_back(s1);
        s1->next.push_back(s2);
        s2->next.push_back(s3);
        scenarioGraph->setInitialNode(firstNode);
    }

private:
    void run() override
    {
        // SECTION 1
        startSection(1);
        // nothing special really, just waiting for more scenarios
        endSection();

        // SECTION 2
        startSection(2);
        buffer->put(23);
        endSection();

        // SECTION 3
        startSection(3);
        // again waiting
        endScenario();
    }

    std::shared_ptr<AbstractBuffer<int>> buffer;
};


/**
 * @brief Thread consommateur 1
 *
 * Il y a 3 sections (4, 5, 6).
 * - Section 4 : éventuellement un "pré-traitement"
 * - Section 5 : get(...)
 * - Section 6 : post-traitement
 */
class ConsumerThread1 : public ObservableThread
{
public:
    ConsumerThread1(std::shared_ptr<AbstractBuffer<int>> buffer, std::string id = "")
        : ObservableThread(std::move(id)),
          buffer(std::move(buffer))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();

        auto firstNode = scenarioGraph->createNode(this, -1);
        auto s4 = scenarioGraph->createNode(this, 4);
        auto s5 = scenarioGraph->createNode(this, 5);
        auto s6 = scenarioGraph->createNode(this, 6);

        firstNode->next.push_back(s4);
        s4->next.push_back(s5);
        s5->next.push_back(s6);
        scenarioGraph->setInitialNode(firstNode);
    }

private:
    void run() override
    {
        // SECTION 4
        startSection(4);
        endSection();

        // SECTION 5
        startSection(5);
        // get item
        int val = buffer->get();
        (void) val;
        endSection();

        // SECTION 6
        startSection(6);
        endScenario();
    }

    std::shared_ptr<AbstractBuffer<int>> buffer;
};


/**
 * @brief Thread consommateur 2
 *
 * Idem que ConsumerThread1 mais avec les sections 7, 8, 9.
 */
class ConsumerThread2 : public ObservableThread
{
public:
    ConsumerThread2(std::shared_ptr<AbstractBuffer<int>> buffer, std::string id = "")
        : ObservableThread(std::move(id)),
          buffer(std::move(buffer))
    {
        scenarioGraph = std::make_unique<ScenarioGraph>();

        auto firstNode = scenarioGraph->createNode(this, -1);
        auto s7 = scenarioGraph->createNode(this, 7);
        auto s8 = scenarioGraph->createNode(this, 8);
        auto s9 = scenarioGraph->createNode(this, 9);

        firstNode->next.push_back(s7);
        s7->next.push_back(s8);
        s8->next.push_back(s9);
        scenarioGraph->setInitialNode(firstNode);
    }

private:
    void run() override
    {
        // SECTION 7
        startSection(7);
        endSection();

        // SECTION 8
        startSection(8);
        // get item
        int val = buffer->get();
        (void) val;
        endSection();

        // SECTION 9
        startSection(9);
        endScenario();
    }

    std::shared_ptr<AbstractBuffer<int>> buffer;
};


/**
 * @brief Modèle pour tester le buffer
 *
 * On crée :
 *  - 1 producteur (3 sections : 1,2,3)
 *  - 2 consommateurs (3 sections chacun : 4, 5, 6 et 7, 8, 9)
 * On lance tout les scénarios jusqu'à la profondeur 9,
 * ce qui permet d'exlporer toutes les permutations d'exécutions
 */
class BufferModel : public PcoModel
{
public:

    bool checkInvariants() override
    {
        return true;
    }

    void build() override
    {
        // shared buffer
        auto buffer = std::make_shared<Buffer2ConsoSemaphore<int>>();

        // add producer
        threads.emplace_back(std::make_unique<ProducerThread>(buffer, "Producer"));

        // add 2 consumer
        threads.emplace_back(std::make_unique<ConsumerThread1>(buffer, "Consumer1"));
        threads.emplace_back(std::make_unique<ConsumerThread2>(buffer, "Consumer2"));

        // 3 threads * 3 sections = 9 possible outcome
        int depth = 9;
        scenarioBuilder = std::make_unique<ScenarioBuilderBuffer>();
        scenarioBuilder->init(threads, depth);
    }

    void preRun(Scenario &scenario) override
    {
        std::cout << "\n===== Nouveau scénario =====\n";
        ScenarioPrint::printScenario(scenario);

    }

    void postRun(Scenario &scenario) override
    {
        std::cout << "\n==== Fin de ce scénario ====\n";
        ScenarioPrint::printScenario(scenario);
        std::cout << std::endl;
    }

    void finalReport() override
    {
        std::cout << "\n======================================\n";
        std::cout << "Tous les scénarios ont été joués.";
        std::cout << "\n======================================\n";
        std::cout << std::endl;

    }
};

#endif // MODELTEMPLATE_H
