#ifndef SCENARIO_H
#define SCENARIO_H

#include <memory>
#include <set>
#include <string>
#include <vector>

// Forward declaration, avoid loop in includes
class ObservableThread;

/// A step in a scenario
typedef struct {
    /// The observable thread that has to start its section
    const ObservableThread *thread;
    /// The section number of this thread
    int number;
} ScenarioPoint;

/// A scenario is a vector of scenario points
/// Its size defines the scenario depth
using Scenario = std::vector<ScenarioPoint>;

///
/// \brief The ScenarioPrint class
///
/// A class offering two static methods, to print a scenario or to create
/// a string from it.
class ScenarioPrint
{
public:
    ///
    /// \brief Prints a scenario on std::cout
    ///
    static void printScenario(const Scenario &scenario);

    ///
    /// \brief Creates a string representing a scenario
    /// \return A string that represents the scenario
    ///
    static std::string toString(const Scenario &scenario);
};


class ScenarioGraph;

///
/// \brief The ScenarioGraphNode class
///
/// This class represents a step in a thread scenario graph. As we can describe
/// a graph, and not only a linear scenario, for a specific thread, it contains
/// a vector of children, each child being attainable after the the section
/// of this specific node.
class ScenarioGraphNode
{
protected:

    friend ScenarioGraph;

    ///
    /// \brief Simple constructor, protected
    /// \param thread A pointer to the observable thread of this step
    /// \param number The section number
    ///
    /// The constructor is protected, so only a ScenarioGraph can
    /// create a ScenarioGraphNode. Ensures a good ownership management.
    ///
    ScenarioGraphNode(const ObservableThread *thread, int number);

public:
    /// The observable thread of this step
    const ObservableThread *thread;

    /// The section number of this step
    int number;

    /// A vector of children, each one being a potentiel next section
    std::vector<ScenarioGraphNode *> next;
};

class ScenarioGraphNode;

///
/// \brief The ScenarioGraph class
///
/// This class represents a scenario-graph of a single thread.
/// It simply embeds a pointer to its first node.
class ScenarioGraph
{
public:
    /// Default constructor
    ScenarioGraph() = default;

    ///
    /// \brief Sets the initial node of the graph
    /// \param node The initial node to be set
    ///
    void setInitialNode(ScenarioGraphNode *node);

    ///
    /// \brief Gets the initial node
    /// \return The initial node
    ///
    ScenarioGraphNode *getFirstNode();

    ///
    /// \brief toDot
    /// \return a string in dot format
    ///
    ///  https://en.wikipedia.org/wiki/DOT_(graph_description_language)
    ///
    /// The string can be put in a file, and then dot run like this:
    ///
    /// dot -Tpng graph.gv -o output.png
    ///
    [[nodiscard]] std::string toDot() const;

    ///
    /// \brief nbScenarios
    /// \return The number of possible scenarios up to a certain depth
    ///
    [[nodiscard]] size_t nbScenarios(int depth) const;

    ///
    /// \brief nbScenarios
    /// \return The number of possible scenarios from a node up to a certain depth
    ///
    [[nodiscard]] static size_t nbScenarios(ScenarioGraphNode *n, int depth);

    // static size_t nbScenarios(std::vector<ScenarioGraphNode *> graphs, int depth);

    ///
    /// \brief Creates a scenario node
    /// \param thread Pointer to the thread owning the node
    /// \param number Unique identifier number of the node
    /// \return A pointer to the newly created node
    ///
    /// This function creates a node and stores a unique_ptr of it in the internal set of nodes.
    ///
    ScenarioGraphNode *createNode(const ObservableThread *thread, int number);

private:

    /// The initial node
    ScenarioGraphNode *m_firstNode{nullptr};

    /// Set of nodes of this graph
    std::set<std::unique_ptr<ScenarioGraphNode>> set;
};


#endif // SCENARIO_H
