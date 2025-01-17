#ifndef OBSERVABLETHREAD_H
#define OBSERVABLETHREAD_H

#include <pcosynchro/pcothread.h>

#include "scenario.h"

class PcoConcurrencyAnalyzer;

///
/// \brief startSection, used to instrumentalize code
/// \param id Id of the section
///
/// This function gets the current observable thread, and calls
/// startSection(thisThread, id) on the current ConcurrencyAnalyzer
///
void startSection(int id);

///
/// \brief endSection, used to instrumentalize code
///
/// This function gets the current observable thread, and calls
/// startSection(thisThread) on the current ConcurrencyAnalyzer
///
void endSection();

///
/// \brief endScenario, used to instrumentalize code
///
/// This function gets the current observable thread, and calls
/// endScenario(thisThread) on the current ConcurrencyAnalyzer
///
void endScenario();

///
/// \brief The ObservableThread class
///
/// This class is used to instanciate threads that have to be observed
/// by the ConcurrencyAnalyzer. It is a pure abstract class, and shall
/// be subclassed. The subclass shall implement the run() method that
/// will be launched thanks to a PcoThread.
class ObservableThread
{
public:

    ///
    /// \brief ObservableThread constructor
    /// \param id Unique Id of the thread
    ///
    /// The constructor registers the thread for future use.
    ///
    explicit ObservableThread(std::string id): id(std::move(id)) {
        std::lock_guard lock(mutex);
        allThreads.push_back(this);
    }

    /// Destructor
    ///
    /// It unregisters the observable thread, and should be called only
    /// after wait() has been called (as a join() would be used)
    ///
    virtual ~ObservableThread() {
        std::lock_guard lock(mutex);
        auto it = allThreads.begin();
        while (it != allThreads.end()) {
            if ((*it) == this) {
                allThreads.erase(it);
                //delete thread;
                return;
            }
            it ++;
        }
        //delete thread;
    }

    ///
    /// \brief Sets the current ConcurrencyAnalyzer
    /// \param analyzer The ConcurrencyAnalyzer that runs the tests
    ///
    void setConcurrencyAnalyzer(PcoConcurrencyAnalyzer *analyzer);


    ///
    /// \brief Starts the thread
    ///
    void start()
    {
        auto t = new PcoThread(&ObservableThread::intRun, this);
        std::lock_guard lock(mutex);
        if (this->thread == nullptr) {
            this->thread = std::unique_ptr<PcoThread>(t);
        }
    }

    ///
    /// \brief Joins on the thread
    ///
    void join()
    {
        if (thread) {
            thread->join();
            std::lock_guard lock(mutex);
            //delete thread
            thread = nullptr;
        }
    }

    [[nodiscard]] ScenarioGraph* getScenarioGraph() const { return scenarioGraph.get();}

    ///
    /// \brief Gets the Id of the thread (the one set through the constructor)
    /// \return The Id of the thread
    ///
    [[nodiscard]] std::string getId() const { return id;}

    ///
    /// \brief Sets the verbosity of sections enters/leaves
    /// \param verbosity true for a full verbosity, false for a quite run.
    ///
    static void setVerbosity(bool verbosity) { verbose = verbosity;}

private:

    ///
    /// \brief Method run by the real thread
    ///
    /// This method shall be implemented in a subclass
    ///
    virtual void run() = 0;

    ///
    /// \brief Gets the currently running observable thread
    /// \return A pointer to the currently running observable thread
    ///
    /// If called from an ObservableThread run() function (or by a
    /// function called by run()), returns a pointer to the ObservableThread.
    /// If called from another standard thread, returns nullptr.
    ///
    static ObservableThread *getCurrentObservable()
    {
        std::lock_guard lock(mutex);
        auto currentThread = PcoThread::thisThread();
        for (const auto& thread : allThreads) {
            if (thread->thread.get() == currentThread) {
                return thread;
            }
        }
        return nullptr;
    }

    ///
    /// \brief startSection wrapper
    /// \param section The section Id
    ///
    void obStartSection(int section);

    ///
    /// \brief endSection wrapper
    ///
    void obEndSection();

    ///
    /// \brief endScenario wrapper
    ///
    void obEndScenario();


    /// Internal method started by the real PcoThread
    void intRun() {
        // We set thread here to be sure it is set when run() starts
        mutex.lock();
        // this->tid = std::this_thread::get_id();
        if (this->thread == nullptr) {
            this->thread = std::unique_ptr<PcoThread>(PcoThread::thisThread());
        }
        mutex.unlock();
        run();
    }

    ///
    /// \brief The current ConcurrencyAnalyzer
    ///
    PcoConcurrencyAnalyzer *analyzer{nullptr};

    ///
    /// \brief The id of the thread, for printing purpose
    ///
    std::string id;

    ///
    /// \brief A static vector storing all the running ObservableThreads
    ///
    static std::vector<ObservableThread*> allThreads;

    ///
    /// \brief A static mutex to protect the allThreads vector
    ///
    static std::mutex mutex;

    ///
    /// \brief The real PcoThread used to run the thread scenario
    ///
    std::unique_ptr<PcoThread> thread {nullptr};

    static bool verbose;

    friend void startSection(int id);
    friend void endSection();
    friend void endScenario();

protected:
    std::unique_ptr<ScenarioGraph> scenarioGraph{nullptr};
};

#endif // OBSERVABLETHREAD_H
