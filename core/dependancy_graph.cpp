#pragma once
#include "dependancy_graph.hpp"
#include "node.hpp"

#include <algorithm>
#include <iostream>
#include <queue>

DependancyGraph::DependancyGraph(
    std::unordered_map<std::string, Node *> &_node_map) {
  node_map = _node_map;
}

void DependancyGraph::add_node(std::string key, Node *val) {
  node_map[key] = val;
  node_map[key]->dgraph = this;
}

void DependancyGraph::display() {
  for (auto it : node_map) {
    std::cout << it.first << ":\n";
    std::cout << "\tprecc: ";
    for (auto it2 : it.second->precc) {
      std::cout << it2 << ",";
    }
    std::cout << "\n\tsuccc: ";
    for (auto it2 : it.second->succ) {
      std::cout << it2 << ",";
    }
    std::cout << "\n";
  }
}

void DependancyGraph::attach() {
  for (auto &it : node_map) {
    // it is ref to each node
    for (auto &it2 : it.second->precc) {
      // it2 is each element in each node of precc

      auto fn_r = std::find(node_map[it2]->succ.begin(),
                            node_map[it2]->succ.end(), it.first);

      if (fn_r == node_map[it2]->succ.end()) {
        // not founds
        node_map[it2]->succ.push_back(it.first);
      }
    }
  }
}

void DependancyGraph::build_indegrees() {
  for (auto &[id, node] : node_map) {
    node->indegree = node->precc.size();
    node->visited = false;
  }
}

void DependancyGraph::apply_pipeline() {
  build_indegrees();

  std::queue<std::string> ready;

  // enqueue nodes with no unmet dependencies
  for (auto &[id, node] : node_map) {
    if (node->indegree == 0) {
      ready.push(id);
    }
  }

  size_t processed = 0;

  while (!ready.empty()) {
    auto id = ready.front();
    ready.pop();

    auto &node = node_map[id];

    if (node->visited)
      continue;

    node->visited = true;
    node->apply_node();
    processed++;

    for (auto &succ : node->succ) {
      auto &s = node_map[succ];
      if (--s->indegree == 0) {
        ready.push(succ);
      }
    }
  }

  // cycle or inconsistency detection
  if (processed != node_map.size()) {
    throw std::logic_error(
        "Pipeline execution failed: graph is not a valid DAG");
  }
}

bool DependancyGraph::cyclicUtil(
    const std::string &u, std::unordered_map<std::string, bool> &visited,
    std::unordered_map<std::string, bool> &recStack) {
  if (recStack[u])
    return true;

  if (visited[u])
    return false;

  visited[u] = true;
  recStack[u] = true;

  for (const std::string &v : node_map[u]->succ) {
    if (cyclicUtil(v, visited, recStack))
      return true;
  }

  recStack[u] = false; // IMPORTANT
  return false;
}

bool DependancyGraph::checkCyclic() {
  std::unordered_map<std::string, bool> visited;
  std::unordered_map<std::string, bool> recStack;

  for (auto &it : node_map) {
    const std::string &u = it.first;
    if (!visited[u] && cyclicUtil(u, visited, recStack))
      return true;
  }
  return false;
}