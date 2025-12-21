#include "sources.hpp"
#include "duck.hpp"
#include <filesystem>

Sources::Sources(const std::string dat,const std::vector<std::string>& _succ,const SourceFileType s,std::string path):Node(dat,{},_succ){
            sfile = s;
            csv_path = path;
            //sources cant have predecessors
            ntype = NodeType::SRC;
            n_file = 1;
}


void Sources::apply_node() {
  MetaDataObject meta = MetaDataObject();
  if (!(std::filesystem::exists(meta.work_dir + "/" + id) &&
        std::filesystem::is_directory(meta.work_dir + "/" + id))) {
    std::filesystem::create_directory(meta.work_dir + "/" + id);
  }
  // construct pquet_path
  std::string pquet_path =
      meta.work_dir + "/" + id + "/" + id + std::to_string(n_file) + ".parquet";

  std::string csv_path_s = meta.work_dir + "/" + csv_path;
  // Extractor::readCSVWriteParquet(csv_path_s,pquet_path);
  Duck::write_csv_parquet(csv_path_s, pquet_path);

  n_file += 1;
  // okay so sources r shob donee
}

