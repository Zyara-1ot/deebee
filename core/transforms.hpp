#pragma once
#include "node.hpp"
#include <string>
#include "node_type.hpp"
#include "transform_materialize.hpp"

class Transform : public Node {
  // okay so lets make it as a source node
private:
  std::string compile_sql(std::string sql, bool fin = false);
public:
  std::string sql;
  TransformMaterialize tfm;
  std::string materialize_table;
  int n_its;
  // introduce parameters depending on tfm
  void apply_node() override;
  Transform(std::string dat, const std::vector<std::string> &_precc,
            const std::vector<std::string> &_succ, std::string sl)
      : Node(dat, _precc, _succ) {
    // constructor
    // get sql string
    ntype = NodeType::TRNS;
    sql = sl;
    n_its = 1;
  }
};