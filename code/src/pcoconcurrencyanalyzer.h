#ifndef PCOCONCURRENCYANALYZER_H
#define PCOCONCURRENCYANALYZER_H

#include <mutex>
#include <condition_variable>

#include "pcomodel.h"

#include "scenario.h"

class ObservableThread;

class PcoConcurrencyAnalyzer
{
public:
    PcoConcurrencyAnalyzer();
    virtual ~PcoConcurrencyAnalyzer();

    ///
    /// \brief Sets the scenario to be played
    /// \param s Scenario to be played
    /// \param nbThreads Total number of threads participating in the play
    ///
    void setScenario(Scenario s, unsigned int nbThreads);

    ///
    /// \brief Restarts a new scenario testing
    /// This function allows to start a new analysis without recreating
    /// a new ConcurrencyAnalyzer object. It should reset the state
    /// of the object except the scenario.
    /// This function is called from setScenario().
    ///
    virtual void start();

    ///
    /// \brief Starts a new control section
    /// \param thread The thread calling the function
    /// \param sectionNumber The section number
    /// This function should be called from the ObservableThread wherever
    /// a new section starts. It should be a blocking function that should
    /// block until the current scenario reaches the section number passed
    /// as parameter.
    ///
    virtual void startSection(ObservableThread *thread, int sectionNumber);

    ///
    /// \brief Indicates the end of a section
    /// \param thread The thread calling the function
    /// This function should be called at the end of a section. However,
    /// a call to startSection() is an implicite end of section in case a
    /// section was already opened.
    ///
    virtual void endSection(ObservableThread *thread);


    ///
    /// \brief endScenario Indicates the end of a scenario
    /// \param thread The thread calling the function
    /// This function should be called at the end of a scenario instead
    /// of a call to endSection.
    ///
    virtual void endScenario(ObservableThread *thread);

    ///
    /// \brief Called by PcoManager when a thread blocks
    /// \param nbBlocked Number of threads blocked on synchro primitives
    ///
    /// This function is called by PcoManager whenever a monitored
    /// semaphore blocks. nbBlocked indicates the number of threads blocked.
    /// Be careful, as the call to this function is made by another thread,
    /// and as such does not necessarily represent the exact current state
    /// of the system.
    ///
    virtual void checkedBlocked(int nbBlocked);

    ///
    /// \brief Indicates that the scenario ended
    /// \return true if the scenario already ended, false else
    ///
    /// This method can be used by the instrumentalized code to know
    /// whether the scenario has aborted or not, so as to act accordingly.
    ///
    virtual bool aborted();

    /// The ending status of the concurrency analyzer
    enum class EndingStatus {
        /// Initial status
        Unknown,
        /// Ended because the entire depth has been simulated
        Depth,
        /// Ended because all observed threads are stuck on synchronization
        /// primitives. It corresponds to a deadlock and indicates there is
        /// an issue with the instrumentalized code.
        Deadlock,
        /// All scenario ended up peacefully
        EndAllScenario,
        /// The scenario ends up in a dead end, so a global state that
        /// can not be reached, but without a global deadlock.
        DeadEnd
    };

    ///
    /// \brief Returns the ending status of the analyzer
    /// \return The current ending status of the analyzer
    ///
    EndingStatus getEndingStatus();

    ///
    /// \brief The current ConcurrencyAnalyzer
    ///
    static PcoConcurrencyAnalyzer *currentAnalyzer;

    void setModel(PcoModel *model);

    const Scenario& getScenario() const;

protected:

    /// The scenario that has to be played
    Scenario scenario;

    /// The number of threads played by the scenario
    unsigned int nbThreads{0};

    /// The ending status
    EndingStatus endingStatus{EndingStatus::Unknown};


    ObservableThread *currentThread{nullptr};
    size_t index{0};
    std::mutex mutex;
    std::condition_variable waiting;
    int nbWaiting{0};
    bool aborting{false};
    int nbRunningThreads;
    PcoModel *model{nullptr};

    ///
    /// \brief Checks the invariants whenever startSection, endSection or endScenario is called
    ///
    /// Displays a message in case the model invariants are not satisfied
    ///
    void checkInvariants();

};

#endif // PCOCONCURRENCYANALYZER_H
