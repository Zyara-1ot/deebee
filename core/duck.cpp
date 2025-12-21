#include <iostream>
#include <vector>

#include "../libduckdb-linux-amd64/duckdb.h"
#include "duck.hpp"

Duck::Duck() {}

Duck::~Duck() noexcept {
  disconnect();
  close();
}

Duck::Duck(std::string dbp) {
  db_path = dbp;
  open_ = false;
  connected_ = false;

  open();
  connect();
}

void Duck::open() {
  if (duckdb_open(db_path.c_str(), &db) == DuckDBError) {
    std::cerr << "Error opening db\n";
    open_ = false;
    return;
  }
  open_ = true;
}

void Duck::connect() {
  if (duckdb_connect(db, &con) == DuckDBError) {
    std::cerr << "Error connecting to database\n"
              << "normie";
    connected_ = false;
  } else {
    connected_ = true;
  }
}

void Duck::disconnect() {
  if (connected_) {
    duckdb_disconnect(&con);
    connected_ = false;
  }
}

void Duck::close() {
  if (open_) {
    duckdb_close(&db);
    open_ = false;
  }
}

bool Duck::is_safe_user_sql(duckdb_connection *con, const std::string &sql) {
  duckdb_result result;
  std::string query_mod = "EXPLAIN " + sql;
  if (duckdb_query(*con, query_mod.c_str(), &result) == DuckDBError) {
    std::cerr << "Error determining SQL safety\n" << sql;
    return false;
  }
  // = con.Query("EXPLAIN" + sql);
  duckdb_destroy_result(&result);
  return true;
}

bool Duck::contains_forbidden(const std::string &sql) {
  // return false;
  static const std::vector<std::string> forbidden = {
      "CREATE", "DROP", "ALTER",  "INSERT", "UPDATE",
      "DELETE", "COPY", "ATTACH", "DETACH", "PRAGMA"};

  std::string upper = sql;
  for (auto &c : upper)
    c = toupper((unsigned char)c);

  for (const auto &kw : forbidden) {
    if (upper.find(kw) != std::string::npos) {
      return true;
    }
  }
  return false;
}

void Duck::write_final(duckdb_connection *con, const std::string &final_table,
                       const std::string &tmp_table) {
  std::string sql;
  /*
      "INSERT INTO " + final_table +
      " SELECT * FROM " + tmp_table;
  */
  duckdb_result result;
  if (!table_exists(con, final_table)) {
    sql = "CREATE TABLE " + final_table + " AS \nSELECT * FROM " + tmp_table +
          " WHERE FALSE";
    if (duckdb_query(*con, sql.c_str(), &result) == DuckDBError) {
      std::cerr << "Error performing query\n" << sql;
      return;
    }
    duckdb_destroy_result(&result);
  }

  sql = "INSERT INTO " + final_table + " SELECT * FROM " + tmp_table;

  if (duckdb_query(*con, sql.c_str(), &result) == DuckDBError) {
    std::cerr << "Error performing query\n" << sql;
    return;
  }
  duckdb_destroy_result(&result);
}

bool Duck::table_exists(duckdb_connection *con, const std::string &table) {
  duckdb_result result;

  std::string query_string =
      "SELECT 1 FROM information_schema.tables WHERE table_name = '" + table +
      "' LIMIT 1";
  if (duckdb_query(*con, query_string.c_str(), &result) == DuckDBError) {
    std::cerr << "Table doesn't exist\n" << table << "\n" << query_string;
    return false;
  }
  bool exists = duckdb_row_count(&result) > 0;
  duckdb_destroy_result(&result);
  return exists;
}

bool Duck::exec(duckdb_connection *con, const std::string &sql) {
  duckdb_result result;
  if (duckdb_query(*con, sql.c_str(), &result) == DuckDBError) {
    std::cerr << "Error performing query\n" << duckdb_result_error(&result);
    return false;
  }
  duckdb_destroy_result(&result);
  return true;
}

void Duck::table_transfer(duckdb_connection *con_dest,
                          const std::string &src_db_path,
                          const std::string &src_table,
                          const std::string &dest_table) {
  if (!is_safe_identifier(src_table) || !is_safe_identifier(dest_table)) {
    throw std::runtime_error("Unsafe table name");
  }

  if (!exec(con_dest, "BEGIN;"))
    return;

  std::string attach_sql = "ATTACH '" + src_db_path + "' AS src;";
  if (!exec(con_dest, attach_sql)) {
    exec(con_dest, "ROLLBACK;");
    return;
  }

  bool detach_needed = true;
  if (!table_exists(con_dest, dest_table)) {

    std::string create_sql = "CREATE TABLE \"" + dest_table +
                             "\" AS SELECT * FROM src." + src_table + ";";
    if (!exec(con_dest, create_sql)) {
      exec(con_dest, "DETACH src;");
      exec(con_dest, "ROLLBACK;");
      return;
    }
    if (detach_needed)
      exec(con_dest, "DETACH src;");
    exec(con_dest, "COMMIT;");

  } else {
    std::string insert_sql = "INSERT INTO main." + dest_table +
                             " SELECT * FROM src." + src_table + ";";
    if (!exec(con_dest, insert_sql)) {
      if (detach_needed)
        exec(con_dest, "DETACH src;");
      exec(con_dest, "ROLLBACK;");
      return;
    }

    if (detach_needed)
      exec(con_dest, "DETACH src;");
    exec(con_dest, "COMMIT;");
  }
}

void Duck::bind_param(duckdb_prepared_statement stmt, int idx, const PARAM &p) {
  if (std::holds_alternative<std::nullptr_t>(p)) {
    duckdb_bind_null(stmt, idx);
  } else if (std::holds_alternative<int64_t>(p)) {
    duckdb_bind_int64(stmt, idx, std::get<int64_t>(p));
  } else if (std::holds_alternative<double>(p)) {
    duckdb_bind_double(stmt, idx, std::get<double>(p));
  } else if (std::holds_alternative<bool>(p)) {
    duckdb_bind_boolean(stmt, idx, std::get<bool>(p));
  } else if (std::holds_alternative<std::string>(p)) {
    duckdb_bind_varchar(stmt, idx, std::get<std::string>(p).c_str());
  }
}

void Duck::exec_params(duckdb_connection *con, const std::string &sql,
                       const PARAMS &params) {
  duckdb_prepared_statement stmt;

  duckdb_prepare(*con, sql.c_str(), &stmt);

  for (int i = 0; i < params.size(); i++) {
    bind_param(stmt, i + 1, params[i]);
  }

  duckdb_result result;
  if (duckdb_execute_prepared(stmt, &result) != DuckDBSuccess) {
    std::cerr << "Insert Error\n";
  }

  duckdb_destroy_result(&result);
  duckdb_destroy_prepare(&stmt);
}

void Duck::write_csv_parquet(const std::string &csv_path,
                             const std::string &pqet_path) {
  duckdb_database db;
  duckdb_connection con;
  duckdb_result result;

  duckdb_prepared_statement stmt;
  int success;
  if (duckdb_open(NULL, &db) != DuckDBError) {
    if (duckdb_connect(db, &con) != DuckDBError) {
      duckdb_result res;
      // Create a table from CSV and write to Parquet
      std::string query =
          "CREATE TABLE my_table AS SELECT * FROM read_csv_auto('" + csv_path +
          "'); " + "COPY my_table TO '" + pqet_path + "' (FORMAT PARQUET);";

      success = duckdb_query(con, query.c_str(), &res);
      if (success != DuckDBSuccess) {
        std::cerr << duckdb_result_error(&res) << std::endl;
        printf("Error executing query\n");
      } else {
        printf("CSV successfully converted to Parquet!\n");
      }
      duckdb_destroy_result(&res);
      duckdb_disconnect(&con);
    }
    duckdb_close(&db);
  }
}

void Duck::write_table_parquet(const std::string &db_path,
                               const std::string &table_name,
                               const std::string &pqet_path) {
  duckdb_database db;
  duckdb_connection con;
  duckdb_result result;

  duckdb_prepared_statement stmt;
  int success;
  if (duckdb_open(db_path.c_str(), &db) != DuckDBError) {
    if (duckdb_connect(db, &con) != DuckDBError) {
      duckdb_result res;
      // Create a table from CSV and write to Parquet
      std::string query = "COPY ( SELECT * FROM \"" + table_name + "\") " +
                          " TO '" + pqet_path + "' (FORMAT PARQUET);";

      success = duckdb_query(con, query.c_str(), &res);
      if (success != DuckDBSuccess) {

        printf("Error executing query\n");
        std::cerr << duckdb_result_error(&res) << std::endl;
      } else {
        printf("CSV successfully converted to Parquet!\n");
      }
      duckdb_destroy_result(&res);
      duckdb_disconnect(&con);
    }
    duckdb_close(&db);
  }
}
