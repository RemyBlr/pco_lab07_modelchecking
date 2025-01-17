#ifndef PCOMODELCHECKER_H
#define PCOMODELCHECKER_H


#include "analyzerwatchdog.h"
#include "pcoconcurrencyanalyzer.h"
#include "pcomodel.h"

///
/// \brief The PcoModelChecker class
///
/// This class is meant to run the model. It does so by running all the scenarios
/// offered by the model. First it builds the model, the run the scenarios and checks
/// for model invariants during the runs. At the end it displays statistics about the execution.
///
/// Typical use:
///
/// \code{cpp}
/// PcoModelImpl model;
/// PcoModelChecker checker;
/// checker.setModel(&model);
/// checker.run();
/// \endcode
///
class PcoModelChecker
{
public:

    ///
    /// \brief Default constructor
    ///
    PcoModelChecker() = default;

    ///
    /// \brief Sets the PcoModel to be used
    /// \param model The PcoModel to be used
    ///
    void setModel(PcoModel *model);

    ///
    /// \brief Runs the model, that is all its scenarios
    ///
    void run();


private:

    ///
    /// \brief Prints an ending status
    /// \param endingStatus status to be printed
    ///
    void printEndingStatus(PcoConcurrencyAnalyzer::EndingStatus endingStatus);

    ///
    /// \brief Prints statistics about all the scenarios.
    ///
    /// It dispays the number of scenarios observed for each ending status.
    ///
    void printStats();

    /// The PcoModel to be run.
    PcoModel *model{nullptr};

    /// A map storing the number of each ending status observed during the run.
    std::map<PcoConcurrencyAnalyzer::EndingStatus, int> endingStatusCounter;

};


#endif // PCOMODELCHECKER_H
