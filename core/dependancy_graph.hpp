#pragma once
#include <list>
#include <string>
#include <unordered_map>



class Node;
class DependancyGraph {
public:
  std::unordered_map<std::string, Node *> node_map;
  std::list<std::string> tasks;
  DependancyGraph() {}
  DependancyGraph(std::unordered_map<std::string, Node *> &_node_map);

  void add_node(std::string key, Node *val);

  void display();
  void attach();

  void build_indegrees();

  void apply_pipeline();
  bool cyclicUtil(const std::string &u,
                  std::unordered_map<std::string, bool> &visited,
                  std::unordered_map<std::string, bool> &recStack);

  bool checkCyclic();
};