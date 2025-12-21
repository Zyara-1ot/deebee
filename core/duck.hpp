#pragma once
#include <string>


#include "params.hpp"

typedef struct _duckdb_connection* duckdb_connection;
typedef struct _duckdb_prepared_statement* duckdb_prepared_statement;
typedef struct _duckdb_database* duckdb_database;



class Duck{
    public:
        Duck();//describe constructor
        Duck(std::string dbp);
        ~Duck() noexcept;
        duckdb_database db;
        duckdb_connection con;

        std::string db_path;

        void disconnect();
        void close();
        void connect();
        void open();

        bool open_;
        bool connected_;
        
        static bool is_safe_user_sql(duckdb_connection *con,const std::string &sql);
        static bool contains_forbidden(const std::string &sql);

        static std::string wrap_temp(const std::string &user_sql,const std::string &tmp_name) {
            return("CREATE TEMP TABLE " + tmp_name + " AS (\n" + user_sql + "\n);");
        }

        static void write_final(duckdb_connection *con,const std::string &final_table,const std::string &tmp_table);
        static bool table_exists(duckdb_connection *con,const std::string &table);
        static bool exec(duckdb_connection *con,const std::string &sql);
        static bool is_safe_identifier(const std::string& identifier){
            //implement later
            return true;
        }
        static void table_transfer(duckdb_connection *con_dest,
                           const std::string &src_db_path,
                           const std::string &src_table,
                           const std::string &dest_table);

        static void bind_param(duckdb_prepared_statement stmt, int idx, const PARAM &p);

        static void exec_params(duckdb_connection *con,const std::string& sql,const PARAMS &params);

        static void write_csv_parquet(const std::string& csv_path,const std::string &pqet_path);

        static void write_table_parquet(const std::string &db_path,const std::string& table_name,const std::string &pqet_path);

        static void write_parquet_table(duckdb_connection *con,const std::string& pqet_path,const std::string& table_name){
            if(!table_exists(con,table_name)){
                std::string query_string = "CREATE TABLE " + table_name + " AS SELECT * FROM read_parquet('" + pqet_path + "');";
                exec(con,query_string);
            }
            else{
                std::string query_string = "INSERT INTO " + table_name + " SELECT * FROM read_parquet('" + pqet_path + "');";
                exec(con,query_string);
            }
        }

};