#pragma once
#include <string>
#include <vector>
#include "node_type.hpp"

class DependancyGraph;
class Node{
    
    //add data
    public:
        std::string id;
        int indegree;
        std::vector<std::string> succ; // succeeding nodes
        std::vector<std::string> precc; // predecessor nodes
        bool visited;//for when pipeline runs

        NodeType ntype;

        DependancyGraph *dgraph;
    
        Node(std::string dat,const std::vector<std::string>& _precc,const std::vector<std::string>& _succ);
        

        //add overloadable method
        virtual void apply_node() = 0;
};