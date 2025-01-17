#include <iostream>
#include <sstream>


#include "scenario.h"
#include "observablethread.h"


std::string ScenarioPrint::toString(const Scenario &scenario)
{
    std::ostringstream stringStream;

    for(const auto& point : scenario) {
        stringStream << "{" << point.thread->getId() << "," << point.number << "} ";

    }
     std::string copyOfStr = stringStream.str();
     return copyOfStr;
}

void ScenarioPrint::printScenario(const Scenario& scenario)
{
    for(const auto& point : scenario) {
        std::cout << "{" << point.thread->getId() << "," << point.number << "} ";

    }
    std::cout << std::endl;
}

ScenarioGraphNode::ScenarioGraphNode(const ObservableThread *thread, int number) :
    thread(thread), number(number)
{}



void ScenarioGraph::setInitialNode(ScenarioGraphNode *node)
{
    m_firstNode = node;
}

ScenarioGraphNode *ScenarioGraph::getFirstNode()
{
    return m_firstNode;
}

ScenarioGraphNode *ScenarioGraph::createNode(const ObservableThread *thread, int number)
{
    // As the constructor is protected, std::make_unique() does not work, even though
    // ScenarioGraph is a friend of ScenarioGraphNode
    auto node = std::unique_ptr<ScenarioGraphNode>(new ScenarioGraphNode(thread, number));
    auto n = node.get();
    set.insert(std::move(node));
    return n;
}


///
/// \brief addToSet
/// \param node
/// \param set
///
/// This function is only used by the Dot generator, to add all nodes of a graph to
/// a set. It does it recursively.
///
static void addToSet(ScenarioGraphNode *node, std::set<ScenarioGraphNode *> &set)
{
    if (set.find(node) == set.end()) {
        set.insert(node);
        for (auto child : node->next) {
            addToSet(child, set);
        }
    }
}

std::string ScenarioGraph::toDot() const
{
    std::set<ScenarioGraphNode *> set;
    if (m_firstNode) {
        addToSet(m_firstNode, set);
    }
    std::stringstream dotString;
    dotString << "digraph mygraph {\n";
    dotString << "rankdir=\"LR\";\n";
    for ( auto node : set) {
        for(auto child : node->next) {
            dotString << node->number << " -> " << child->number << ";\n";
        }
    }
    dotString << "}";
    return dotString.str();
}


size_t ScenarioGraph::nbScenarios(int depth) const
{
    size_t result = 0;
    for (auto child : m_firstNode->next) {
        result += nbScenarios(child, depth);
    }
    return result;
}


size_t ScenarioGraph::nbScenarios(ScenarioGraphNode *n, int depth)
{
    size_t result = 0;
    if (depth == 0) {
        return 1;
    }
    if (n->next.empty()) {
        return 1;
    }
    for (auto child : n->next) {
        result += nbScenarios(child, depth - 1);
    }
    return result;
}
