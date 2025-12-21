#include "node.hpp"

Node::Node(std::string dat,const std::vector<std::string>& _precc,const std::vector<std::string>& _succ){
    id = dat;
    //TODO: Look into problems that may arise here later
    precc = _precc;
    succ = _succ;
    ntype = NodeType::NONE;
    visited = false;
    indegree = 0;
    dgraph = nullptr;
}