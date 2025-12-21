#include <arrow/io/interfaces.h>
#include <arrow/io/type_fwd.h>

#include <arrow/status.h>
#include <cctype>
#include <cstddef>
#include <iostream>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>
#include <algorithm>
#include <queue>
#include <list>
#include <regex>
#include <filesystem>

#include <arrow/api.h>
#include <arrow/csv/api.h>
#include <arrow/io/file.h>

#include <parquet/arrow/writer.h>

#include "parser/toml.hpp"


#include "libduckdb-linux-amd64/duckdb.h"



//TODO: Add actual operational backend for db (parsing capabilities)

//TODO: Add support for other file types later
using PARAM = std::variant<
    std::nullptr_t,
    int64_t,
    double,
    bool,
    std::string
>;

using PARAMS = std::vector<PARAM>;

enum SourceFileType {CSV,KFKA};
enum TransformMaterialize {TEMP,TABLE,VIEW,PQET,FINAL};
enum LoadFormat {PARAQUET,DUCKDB};
enum NodeType {NONE,SRC,TRNS,LOAD};

//should be generated at start of process

class DependancyGraph;
struct MetaData{
    std::string work_dir;
};

MetaData metadata;
class MetaDataObject{
    //stores all info about present execution
    public:
        std::string work_dir;
        MetaDataObject(){
            //get data from file
            bool metaDataEx = false;
            work_dir = metadata.work_dir;
            //find existence of file/folders
            //work_dir = std::filesystem::current_path();
            if(metaDataEx){
                
            }
            else{
                //create metadata files
            }
        }
};

class Duck{
    public:
        Duck() = delete;
        static bool is_safe_user_sql(duckdb_connection *con,const std::string &sql){
            duckdb_result result;
            std::string query_mod = "EXPLAIN " + sql;
            if(duckdb_query(*con, query_mod.c_str(), &result) == DuckDBError){ 
                std::cerr << "Error determining SQL safety\n" << sql; 
                return false; 
            } 
            // = con.Query("EXPLAIN" + sql);
            duckdb_destroy_result(&result);
            return true;
        }
        static bool contains_forbidden(const std::string &sql) {
            //return false;
            static const std::vector<std::string> forbidden = {
                "CREATE", "DROP", "ALTER", "INSERT", "UPDATE", "DELETE",
                "COPY", "ATTACH", "DETACH", "PRAGMA"
            };

            std::string upper = sql;
            for(auto& c : upper) c = toupper((unsigned char)c);
            
            for (const auto &kw : forbidden) {
                if (upper.find(kw) != std::string::npos) {
                    return true;
                }
            }
            return false;
        }

        static std::string wrap_temp(const std::string &user_sql,const std::string &tmp_name) {
            return("CREATE TEMP TABLE " + tmp_name + " AS (\n" + user_sql + "\n);");
        }

        static void write_final(duckdb_connection *con,const std::string &final_table,const std::string &tmp_table) {
            std::string sql;
            /*
                "INSERT INTO " + final_table +
                " SELECT * FROM " + tmp_table;
            */
            duckdb_result result;
            if(!table_exists(con,final_table)){
                sql = "CREATE TABLE " + final_table + " AS \nSELECT * FROM " + tmp_table + " WHERE FALSE";
                if(duckdb_query(*con, sql.c_str(), &result) == DuckDBError){ 
                    std::cerr << "Error performing query\n" << sql; 
                    return; 
                }
                duckdb_destroy_result(&result);
            }
            
            sql = "INSERT INTO " + final_table +
                " SELECT * FROM " + tmp_table;
            
            if(duckdb_query(*con, sql.c_str(), &result) == DuckDBError){ 
                std::cerr << "Error performing query\n" << sql; 
                return; 
            }
            duckdb_destroy_result(&result);
        }
        static bool table_exists(duckdb_connection *con,const std::string &table) {
            duckdb_result result;
            
            std::string query_string = "SELECT 1 FROM information_schema.tables WHERE table_name = '" + table + "' LIMIT 1";
            if(duckdb_query(*con, query_string.c_str(), &result) == DuckDBError){ 
                std::cerr << "Table doesn't exist\n" << table << "\n" << query_string; 
                return false; 
            }
            bool exists = duckdb_row_count(&result) > 0;
            duckdb_destroy_result(&result);
            return exists;
        }
        static bool exec(duckdb_connection *con,const std::string &sql){
            duckdb_result result;
            if(duckdb_query(*con,sql.c_str(),&result) == DuckDBError){
                std::cerr << "Error performing query\n" << duckdb_result_error(&result); 
                return false; 
            }
            duckdb_destroy_result(&result);
            return true;
        }
        static bool is_safe_identifier(const std::string& identifier){
            //implement later
            return true;
        }
        static void table_transfer(duckdb_connection *con_dest,
                           const std::string &src_db_path,
                           const std::string &src_table,
                           const std::string &dest_table) {
            if (!is_safe_identifier(src_table) || !is_safe_identifier(dest_table)) {
                throw std::runtime_error("Unsafe table name");
            }

            if(!exec(con_dest, "BEGIN;")) return;

            std::string attach_sql = "ATTACH '" + src_db_path + "' AS src;";
            if(!exec(con_dest, attach_sql)) {
                exec(con_dest, "ROLLBACK;");
                return;
            }

            bool detach_needed = true;
            if(!table_exists(con_dest,dest_table)) {

                std::string create_sql = "CREATE TABLE \"" + dest_table +
                                        "\" AS SELECT * FROM src." + src_table + ";";
                if(!exec(con_dest, create_sql)) {
                    exec(con_dest,"DETACH src;");
                    exec(con_dest,"ROLLBACK;");
                    return;
                }
                if(detach_needed) exec(con_dest,"DETACH src;");
                exec(con_dest,"COMMIT;");

            }
            else{
                std::string insert_sql = "INSERT INTO main." + dest_table +
                                    " SELECT * FROM src." + src_table + ";";
                if(!exec(con_dest, insert_sql)) {
                    if(detach_needed) exec(con_dest,"DETACH src;");
                    exec(con_dest,"ROLLBACK;");
                    return;
                }

                if(detach_needed) exec(con_dest,"DETACH src;");
                exec(con_dest,"COMMIT;");
            }
    
    
        }


        static void bind_param(duckdb_prepared_statement stmt, int idx, const PARAM &p) {
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

        static void exec_params(duckdb_connection *con,const std::string& sql,const PARAMS &params){
            duckdb_prepared_statement stmt;

            duckdb_prepare(*con,sql.c_str(),&stmt);
            
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

        static void write_csv_parquet(const std::string& csv_path,const std::string &pqet_path){
            duckdb_database db;
            duckdb_connection con;
            duckdb_result result;

            duckdb_prepared_statement stmt;
            int success;
            if(duckdb_open(NULL,&db) != DuckDBError){
                if (duckdb_connect(db, &con) != DuckDBError) {
                    duckdb_result res;
                    // Create a table from CSV and write to Parquet
                    std::string query = "CREATE TABLE my_table AS SELECT * FROM read_csv_auto('" + csv_path + "'); " +
                        "COPY my_table TO '" + pqet_path + "' (FORMAT PARQUET);";

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

        static void write_table_parquet(const std::string &db_path,const std::string& table_name,const std::string &pqet_path){
            duckdb_database db;
            duckdb_connection con;
            duckdb_result result;

            duckdb_prepared_statement stmt;
            int success;
            if(duckdb_open(db_path.c_str(),&db) != DuckDBError){
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

class Extractor{
    public:
        Extractor() = delete;
        static arrow::Status readCSVWriteParquet(std::string& csv_path,std::string& pqt_path){
            arrow::io::IOContext io_context = arrow::io::default_io_context();
            std::shared_ptr<arrow::io::InputStream> input;

            ARROW_ASSIGN_OR_RAISE(input,arrow::io::ReadableFile::Open(csv_path));

            auto read_options = arrow::csv::ReadOptions::Defaults();
            auto parse_options = arrow::csv::ParseOptions::Defaults();
            auto convert_options = arrow::csv::ConvertOptions::Defaults();

            // Instantiate TableReader from input stream and options
            auto maybe_reader = arrow::csv::TableReader::Make(
                io_context,input,read_options,parse_options,convert_options);

            if (!maybe_reader.ok()) {
                // Handle TableReader instantiation error...
            }
            std::shared_ptr<arrow::csv::TableReader> reader = *maybe_reader;

            // Read table from CSV file
            auto maybe_table = reader->Read();
            if (!maybe_table.ok()) {
                // Handle CSV read error
                // (for example a CSV syntax error or failed type conversion)
            }
            std::shared_ptr<arrow::Table> table = *maybe_table;
            
            using parquet::ArrowWriterProperties;
            using parquet::WriterProperties;

            //ARROW_ASSIGN_OR_RAISE(std::shared_ptr<arrow::Table> table, GetTable());

            // Choose compression
            std::shared_ptr<WriterProperties> props =
                WriterProperties::Builder().compression(arrow::Compression::SNAPPY)->build();

            // Opt to store Arrow schema for easier reads back into Arrow
            std::shared_ptr<ArrowWriterProperties> arrow_props =
                ArrowWriterProperties::Builder().store_schema()->build();

            std::shared_ptr<arrow::io::FileOutputStream> outfile;
            ARROW_ASSIGN_OR_RAISE(outfile, arrow::io::FileOutputStream::Open(pqt_path));

            ARROW_RETURN_NOT_OK(parquet::arrow::WriteTable(*table.get(),
                                               arrow::default_memory_pool(), outfile,
                                               /*chunk_size=*/3, props, arrow_props));
            return arrow::Status::OK();
        }
};


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
    
        Node(std::string dat,const std::vector<std::string>& _precc,const std::vector<std::string>& _succ){
            id = dat;
            //TODO: Look into problems that may arise here later
            precc = _precc;
            succ = _succ;
            ntype = NONE;
            visited = false;
            indegree = 0;
            dgraph = nullptr;
        }
        

        //add overloadable method
        virtual void apply_node(){
            
        }
};

class DependancyGraph{
    public:
        std::unordered_map<std::string, Node*> node_map;
        std::list<std::string> tasks;
        DependancyGraph(){

        }
        DependancyGraph(std::unordered_map<std::string, Node*>& _node_map){
            node_map = _node_map;
        }

        void add_node(std::string key,Node *val){
            node_map[key] = val;
            node_map[key]->dgraph = this;
        }

        void display(){
            for(auto it : node_map){
                std::cout << it.first << ":\n";
                std::cout << "\tprecc: ";
                for(auto it2 : it.second->precc){
                    std::cout << it2 << ",";
                } 
                std::cout << "\n\tsuccc: ";
                for(auto it2 : it.second->succ){
                    std::cout << it2 << ",";
                }
                std::cout << "\n";
            }
        }
        void attach(){
            for(auto& it : node_map){
                //it is ref to each node
                for(auto& it2 : it.second->precc){
                    //it2 is each element in each node of precc

                    auto fn_r = std::find(node_map[it2]->succ.begin(),node_map[it2]->succ.end(),it.first);

                    if(fn_r == node_map[it2]->succ.end()){
                        //not founds
                        node_map[it2]->succ.push_back(it.first);
                        
                    }
                }
            }
        }


        void build_indegrees() {
            for (auto& [id, node] : node_map) {
                node->indegree = node->precc.size();
                node->visited = false;
            }
        }
        void apply_pipeline() {
            build_indegrees();

            std::queue<std::string> ready;

            // enqueue nodes with no unmet dependencies
            for (auto& [id, node] : node_map) {
                if (node->indegree == 0) {
                    ready.push(id);
                }
            }

            size_t processed = 0;

            while (!ready.empty()) {
                auto id = ready.front();
                ready.pop();

                auto& node = node_map[id];

                if (node->visited)
                    continue;

                node->visited = true;
                node->apply_node();
                processed++;

                for (auto& succ : node->succ) {
                    auto& s = node_map[succ];
                    if (--s->indegree == 0) {
                        ready.push(succ);
                    }
                }
            }

            // cycle or inconsistency detection
            if (processed != node_map.size()) {
                throw std::logic_error(
                    "Pipeline execution failed: graph is not a valid DAG"
                );
            }
        }
        bool cyclicUtil(const std::string &u,std::unordered_map<std::string,bool> &visited,std::unordered_map<std::string,bool> &recStack){
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

            recStack[u] = false;   // IMPORTANT
            return false;
        }

        bool checkCyclic(){
            std::unordered_map<std::string,bool> visited;
            std::unordered_map<std::string,bool> recStack;

            for (auto &it : node_map) {
                const std::string &u = it.first;
                if (!visited[u] && cyclicUtil(u, visited, recStack))
                    return true;
            }
            return false;
        }
};


class Sources : public Node{
    public:
        SourceFileType sfile;
        
        //only csv supported till now
        std::string csv_path;
        std::string csv_delim;
        bool csv_header;
        
        int n_file;//no. of files
        
        //need no input we do for sources
        void apply_node() override{
            MetaDataObject meta = MetaDataObject();
            if(!(std::filesystem::exists(meta.work_dir + "/" + id) && std::filesystem::is_directory(meta.work_dir + "/" + id))){
                std::filesystem::create_directory(meta.work_dir + "/" + id);
            }
            //construct pquet_path  
            std::string pquet_path = meta.work_dir + "/" + id + "/" + id + std::to_string(n_file) + ".parquet";

            std::string csv_path_s = meta.work_dir + "/" + csv_path;
            //Extractor::readCSVWriteParquet(csv_path_s,pquet_path);
            Duck::write_csv_parquet(csv_path_s,pquet_path);
            
            n_file += 1;
            //okay so sources r shob donee
        }

        Sources(const std::string dat,const std::vector<std::string>& _succ,const SourceFileType s,std::string path):Node(dat,{},_succ){
            sfile = s;
            csv_path = path;
            //sources cant have predecessors
            ntype = SRC;
            n_file = 1;
        }
};

class Transform : public Node{
    //okay so lets make it as a source node
    private:
        std::string compile_sql(std::string sql,bool fin = false) {
            if(dgraph == NULL){
                throw std::runtime_error("null dgraph");
            }

            std::smatch m;
            std::regex ref_re(R"(\{\{ref\(([^)]*)\)\}\})");
            MetaDataObject meta = MetaDataObject();
            while (std::regex_search(sql, m, ref_re)) {
                std::string name = m[1];

                auto it = find(precc.begin(),precc.end(),name);
                
                if (it == precc.end()) {
                    for(int i = 0;i < precc.size();i++){
                        std::cout<<precc[i] << ",";
                    }
                    std::cout << "end" << "\n";
                    throw std::runtime_error("Unknown ref: " + name);
                }
                
                std::string replacement;

                if(dgraph->node_map[name]->ntype == SRC || (dgraph->node_map[name]->ntype == TRNS && ((Transform *)dgraph->node_map[name])->tfm == PQET)){
                    //calculate working directory
                    replacement = "read_parquet('" + meta.work_dir + "/" + name + "/*.parquet"  + "')";
                }
                else{
                    if(dgraph->node_map[name]->ntype == TRNS && ((Transform *)dgraph->node_map[name])->tfm == TABLE){
                        replacement = name;//materialize_table was here before
                        if(fin){
                            //required for final materialize

                            replacement = "src." + name;
                        }
                    }
                    else{
                        throw std::runtime_error("sql compile error");
                    }
                } 
                
                sql.replace(m.position(0), m.length(0), replacement);
            }
            //add other prepend and append stuff to sql query based on state somewhere else
            return sql;
        }
    public:
        std::string sql;
        TransformMaterialize tfm;
        std::string materialize_table;
        int n_its;
        //introduce parameters depending on tfm
        void apply_node(){
            //do something to apply transform using sql
            //since it is always saved, we need a way to cache it or something

            //get the paraquet as argument (we can obtain that from the precc and going to those to get the data)
            std::string comp_sql = compile_sql(sql);
            //[x] 1. get ref calls in sql, extract data within ref
            //[x] 2. check what it points to.. if table then substitute table read stuff else substitute paraquet read stuff
            duckdb_database db;
            duckdb_connection con;
        
            MetaDataObject meta = MetaDataObject();
            std::string db_path = meta.work_dir + "/proc.db";
            if(duckdb_open(db_path.c_str(),&db) == DuckDBError){
                std::cerr << "Error opening db\n";
                return;
            }
            
            if(duckdb_connect(db,&con) == DuckDBError){
                std::cerr << "Error connecting to database\n" << "normie";
            }


            //contains_forbidden
            if(Duck::contains_forbidden(comp_sql)){
                std::cerr << "Invalid SQL Operations\n";
                return;
            }

            //safe
            if(!Duck::is_safe_user_sql(&con,comp_sql)){
                std::cerr << "Is not valid SQL\n";
                return;
            }

            //wraptemp
            comp_sql = Duck::wrap_temp(comp_sql,"tmp");
            
            


            Duck::exec(&con,comp_sql);

            Duck::write_final(&con,materialize_table,"tmp");
            duckdb_disconnect(&con);
            duckdb_close(&db);
            if(tfm == FINAL){
                //FINAL
                std::string fin_path = meta.work_dir + "/final/" + materialize_table + "/" + materialize_table + std::to_string(n_its) + ".parquet";
                if(!(std::filesystem::exists(meta.work_dir + "/final") && std::filesystem::is_directory(meta.work_dir + "/final"))){
                    std::filesystem::create_directory(meta.work_dir + "/final");
                    std::filesystem::create_directory(meta.work_dir + "/final/" + materialize_table);
                }

                if(!(std::filesystem::exists(meta.work_dir + "/final/" + materialize_table) && std::filesystem::is_directory(meta.work_dir + "/final/" + materialize_table))){
                    std::filesystem::create_directory(meta.work_dir + "/final/" + materialize_table);
                }


                Duck::write_table_parquet(db_path,materialize_table,fin_path);
            
            }
            
            

            if(tfm == FINAL){
                duckdb_database db_f;
                duckdb_connection con_c;

                std::string pqet_path_f = meta.work_dir + "/final/" + materialize_table + "/*.parquet";
                if(!(std::filesystem::exists(meta.work_dir + "/final") && std::filesystem::is_directory(meta.work_dir + "/final"))){
                    std::filesystem::create_directory(meta.work_dir + "/final");
                }
                std::string fin_path = meta.work_dir + "/final/final.duckdb";


                if(duckdb_open(fin_path.c_str(),&db_f) == DuckDBError){
                    std::cerr << "Error opening db\n";
                    return;
                }
            
                if(duckdb_connect(db_f,&con_c) == DuckDBError){
                    std::cerr << "Error connecting to database\n" << "final";
                    return;
                }

                //Duck::table_transfer(&con_c, db_path,materialize_table,materialize_table);
                
                Duck::write_parquet_table(&con_c, pqet_path_f,materialize_table);
                duckdb_disconnect(&con_c);
                duckdb_close(&db_f);

            }

            if(duckdb_open(db_path.c_str(),&db) == DuckDBError){
                std::cerr << "Error opening db\n";
                return;
            }
            
            if(duckdb_connect(db,&con) == DuckDBError){
                std::cerr << "Error connecting to database\n";
            }


            std::string table_dropper = "DROP TABLE IF EXISTS tmp;";
            Duck::exec(&con,table_dropper);

            duckdb_disconnect(&con);
            duckdb_close(&db);
            //need to add : error handling
        }
        Transform(std::string dat,const std::vector<std::string>& _precc,const std::vector<std::string>& _succ,std::string sl):Node(dat,_precc,_succ){
            //constructor
            //get sql string 
            ntype = TRNS;
            sql = sl;
            n_its = 1;
        }
};

class MainClass{
    public:
        std::string work_dir;

        toml::table config;
        toml::node_view<toml::node> settings;
        toml::node_view<toml::node> sources;
        toml::node_view<toml::node> transforms;
        
        DependancyGraph *dag;
        void run_stuff(){
            if(dag != nullptr){
                dag->apply_pipeline();
            }
        }
        MainClass(){
            work_dir = metadata.work_dir;
            std::cout << work_dir << "\n";
            config = toml::parse_file(work_dir + "/config.toml");
            settings = config["settings"];
            sources = config["sources"];
            transforms = config["transforms"];
            dag = new DependancyGraph();

            Sources *src = nullptr;
            if(sources.is_table()){
                for(auto it : *sources.as_table()){
                    //it is each source

                    if(it.second.is_table()){
            
                        std::string csv_path = config["sources"][it.first]["path"].value_or<std::string>("");
                        std::string csv_delim = config["sources"][it.first]["delimiter"].value_or<std::string>("");
                        bool csv_header = config["sources"][it.first]["header"].value_or<bool>(true);

                        //add these to DAG.. sources are all starting nodes.
                        src = new Sources(it.first.data(),std::vector<std::string>{},CSV,csv_path);
                        if(src != nullptr){
                            dag->add_node(it.first.data(),src);
                        }

                        src = nullptr;
                    }
                }
            }

            Transform *tfr = nullptr;
            if(transforms.is_table()){
                for(auto it : *transforms.as_table()){
                    if(it.second.is_table()){
                        //set materialize later
                        tfr = nullptr;
                        std::string sql = config["transforms"][it.first]["sql"].value_or<std::string>("");
                        std::vector<std::string> depends;
                        toml::array tomlArr = *config["transforms"][it.first]["depends"].as_array();
                        std::string materialize = config["transforms"][it.first]["materialize"].value_or<std::string>("");
                        for(const auto &item : tomlArr){
                            depends.push_back(item.as_string()->get());
                        }


                        //now all is extracted.
                        tfr = new Transform(it.first.data(),depends,std::vector<std::string>{},sql);
                        if(tfr != nullptr){
                            if(materialize == "final"){
                                tfr->tfm = FINAL;
                                tfr->materialize_table = config["transforms"][it.first]["table_name"].value_or<std::string>("");
                            }
                            else{
                                tfr->tfm = TABLE;
                                tfr->materialize_table = it.first.data();
                            }
                            dag->add_node(it.first.data(),tfr);
                        }
                    }
                }
            }

            dag->attach();
            dag->display();

        }

};
int main(){
    metadata.work_dir = std::filesystem::current_path();
    MainClass *m = new MainClass();
    m->run_stuff();
}