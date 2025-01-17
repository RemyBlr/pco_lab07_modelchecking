#ifndef PCOMODEL_H
#define PCOMODEL_H

#include "observablethread.h"
#include "scenariobuilder.h"

class PcoModel
{
public:

    ///
    /// \brief Default virtual destructor
    ///
    virtual ~PcoModel() = default;

    ///
    /// \brief Builds the threads and scenariobuilder
    ///
    /// This function shall be overriden and contain code to build the threads vector
    /// and the scenarioBuilder.
    ///
    /// Example:
    /// \code{cpp}
    ///
    /// void build() override {
    ///     threads.emplace_back(std::make_unique<ThreadA>("1"));
    ///     threads.emplace_back(std::make_unique<ThreadB>("2"));
    ///     threads.emplace_back(std::make_unique<ThreadC>("3"));
    ///
    ///     scenarioBuilder = std::make_unique<ScenarioBuilderBuffer>();
    ///     scenarioBuilder->init(threads, 9);
    /// }
    /// \endcode
    ///
    virtual void build() = 0;

    ///
    /// \brief Function called by the model checker before running a scenario.
    /// \param scenario The scenario that will be run.
    /// This function is called before the scenario starts and can be overriden
    /// is specific tasks have to be carried out at that time. By default it
    /// does nothing, so there is no need to override it if not useful.
    ///
    virtual void preRun(Scenario &/*scenario*/) {}

    ///
    /// \brief Function called by the model checker after running a scenario.
    /// \param scenario The scenario that has been run.
    /// This function is called after the end of the scenario and can be overriden
    /// is specific tasks have to be carried out at that time, for instance to check
    /// the status of the system at the end of the scenario. By default it
    /// does nothing, so there is no need to override it if not useful.
    ///
    virtual void postRun(Scenario &/*scenario*/) {}

    ///
    /// \brief Function called at the end of all scenario.
    ///
    /// This function is called by the model checker after all scenario have been
    /// carried out. It allows for instance to wrap up a summary.
    /// By default it does nothing, so there is no need to override it if not useful.
    ///
    virtual void finalReport() {}

    ///
    /// \brief Function to check invariants of the model.
    /// \return true if all invariants stand, false else.
    ///
    /// This function is automatically called by startSection(), endSection() and endScenario() functions.
    /// By default it returns true, but it can be overriden if model invariants are meaningful.
    ///
    virtual bool checkInvariants() {return true;}

    ///
    /// \brief Returns a pointer to the scenario builder of the model
    /// \return A pointer to the scenario builder of the model.
    ///
    /// This function should normally not be overriden.
    ///
    ScenarioBuilderInterface* getScenarioBuilder();

    ///
    /// \brief Gets the vector of observable threads of the model.
    /// \return The vector of observable threads.
    ///
    /// This function should normally not be overriden.
    ///
    const std::vector<std::unique_ptr<ObservableThread> >& getThreads();

protected:

    ///
    /// \brief Vector of ObservablThread.
    ///
    /// This vector should be filled within the build() function.
    ///
    std::vector<std::unique_ptr<ObservableThread> > threads;

    ///
    /// \brief Scenario builder of the model.
    ///
    /// This ScenarioBuilder should be built in the build() function.
    ///
    std::unique_ptr<ScenarioBuilderInterface> scenarioBuilder{nullptr};
};

#endif // PCOMODEL_H
