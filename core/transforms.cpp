#include "transforms.hpp"
#include "dependancy_graph.hpp"
#include "duck.hpp"
#include "meta.hpp"

#include <filesystem>
#include <iostream>
#include <regex>

std::string Transform::compile_sql(std::string sql, bool fin) {
  if (dgraph == NULL) {
    throw std::runtime_error("null dgraph");
  }

  std::smatch m;
  std::regex ref_re(R"(\{\{ref\(([^)]*)\)\}\})");
  //MetaDataObject meta = MetaDataObject();
  while (std::regex_search(sql, m, ref_re)) {
    std::string name = m[1];

    auto it = find(precc.begin(), precc.end(), name);

    if (it == precc.end()) {
      for (int i = 0; i < precc.size(); i++) {
        std::cout << precc[i] << ",";
      }
      std::cout << "end" << "\n";
      throw std::runtime_error("Unknown ref: " + name);
    }

    std::string replacement;

    if (dgraph->node_map[name]->ntype == NodeType::SRC ||
        (dgraph->node_map[name]->ntype == NodeType::TRNS &&
         ((Transform *)dgraph->node_map[name])->tfm ==
             TransformMaterialize::PQET)) {
      // calculate working directory
      replacement =
          "read_parquet('" + meta().work_dir + "/" + name + "/*.parquet" + "')";
    } else {
      if (dgraph->node_map[name]->ntype == NodeType::TRNS &&
          ((Transform *)dgraph->node_map[name])->tfm ==
              TransformMaterialize::TABLE) {
        replacement = name; // materialize_table was here before
        if (fin) {
          // required for final materialize

          replacement = "src." + name;
        }
      } else {
        throw std::runtime_error("sql compile error");
      }
    }

    sql.replace(m.position(0), m.length(0), replacement);
  }
  // add other prepend and append stuff to sql query based on state somewhere
  // else
  return sql;
}

void Transform::apply_node() {
  // do something to apply transform using sql
  // since it is always saved, we need a way to cache it or something

  // get the paraquet as argument (we can obtain that from the precc and going
  // to those to get the data)
  std::string comp_sql = compile_sql(sql);
  //[x] 1. get ref calls in sql, extract data within ref
  //[x] 2. check what it points to.. if table then substitute table read stuff
  // else substitute paraquet read stuff

  // duckdb_database db;
  // duckdb_connection con;

  //MetaDataObject meta = MetaDataObject();

  std::string db_path = meta().work_dir + "/proc.db";
  Duck db = Duck(db_path);

  // contains_forbidden
  if (Duck::contains_forbidden(comp_sql)) {
    std::cerr << "Invalid SQL Operations\n";
    return;
  }

  // safe
  if (!Duck::is_safe_user_sql(&db.con, comp_sql)) {
    std::cerr << "Is not valid SQL\n";
    return;
  }

  // wraptemp
  comp_sql = Duck::wrap_temp(comp_sql, "tmp");

  Duck::exec(&db.con, comp_sql);


if (tfm == TransformMaterialize::TABLE) {
    Duck::write_final(&db.con, materialize_table, "tmp");
}
else if (tfm == TransformMaterialize::PQET) {
    std::string pqet_dir =
        meta().work_dir + "/" + materialize_table;

    if (!std::filesystem::exists(pqet_dir)) {
        std::filesystem::create_directory(pqet_dir);
    }

    std::string pqet_path =
        pqet_dir + "/" + materialize_table +
        std::to_string(n_its) + ".parquet";

    Duck::write_table_parquet(db_path, "tmp", pqet_path);
}



  std::string table_dropper = "DROP TABLE IF EXISTS tmp;";
  Duck::exec(&db.con, table_dropper);

  db.disconnect();
  db.close();
  if (tfm == TransformMaterialize::FINAL) {
    // FINAL
    std::string fin_path = meta().work_dir + "/final/" + materialize_table + "/" +
                           materialize_table + std::to_string(n_its) +
                           ".parquet";
    if (!(std::filesystem::exists(meta().work_dir + "/final") &&
          std::filesystem::is_directory(meta().work_dir + "/final"))) {
      std::filesystem::create_directory(meta().work_dir + "/final");
      std::filesystem::create_directory(meta().work_dir + "/final/" +
                                        materialize_table);
    }

    if (!(std::filesystem::exists(meta().work_dir + "/final/" +
                                  materialize_table) &&
          std::filesystem::is_directory(meta().work_dir + "/final/" +
                                        materialize_table))) {
      std::filesystem::create_directory(meta().work_dir + "/final/" +
                                        materialize_table);
    }

    Duck::write_table_parquet(db_path, materialize_table, fin_path);
  }

  if (tfm == TransformMaterialize::FINAL) {
    // duckdb_database db_f;
    // duckdb_connection con_c;

    std::string pqet_path_f =
        meta().work_dir + "/final/" + materialize_table + "/*.parquet";
    if (!(std::filesystem::exists(meta().work_dir + "/final") &&
          std::filesystem::is_directory(meta().work_dir + "/final"))) {
      std::filesystem::create_directory(meta().work_dir + "/final");
    }
    std::string fin_path = meta().work_dir + "/final/final.duckdb";

    Duck db_f = Duck(fin_path);
    // Duck::table_transfer(&con_c,
    // db_path,materialize_table,materialize_table);

    Duck::write_parquet_table(&db_f.con, pqet_path_f, materialize_table);
    db_f.disconnect();
    db_f.close();
  }
  // need to add : error handling
}