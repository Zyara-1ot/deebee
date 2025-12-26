#include "toml_parse.hpp"
#include "../core/meta.hpp"
#include "../core/sources.hpp"
#include "../core/transforms.hpp"

#include <iostream>

void TomlParse::run_stuff() {
  if (dag != nullptr) {
    dag->apply_pipeline();
  }
}

TomlParse::TomlParse(const std::string &workdir) {
  meta().work_dir = workdir;
  work_dir = meta().work_dir;
  std::cout << work_dir << "\n";
  config = toml::parse_file(work_dir + "/config.toml");
  settings = config["settings"];
  sources = config["sources"];
  transforms = config["transforms"];
  dag = new DependancyGraph();

  Sources *src = nullptr;
  if (sources.is_table()) {
    for (auto it : *sources.as_table()) {
      // it is each source

      if (it.second.is_table()) {

        std::string csv_path =
            config["sources"][it.first]["path"].value_or<std::string>("");
        std::string csv_delim =
            config["sources"][it.first]["delimiter"].value_or<std::string>("");
        bool csv_header =
            config["sources"][it.first]["header"].value_or<bool>(true);

        // add these to DAG.. sources are all starting nodes.
        src = new Sources(it.first.data(), std::vector<std::string>{},
                          SourceFileType::CSV, csv_path);
        if (src != nullptr) {
          dag->add_node(it.first.data(), src);
        }

        src = nullptr;
      }
    }
  }

  Transform *tfr = nullptr;
  if (transforms.is_table()) {
    for (auto it : *transforms.as_table()) {
      if (it.second.is_table()) {
        // set materialize later
        tfr = nullptr;
        std::string sql =
            config["transforms"][it.first]["sql"].value_or<std::string>("");
        std::vector<std::string> depends;
        toml::array tomlArr =
            *config["transforms"][it.first]["depends"].as_array();
        std::string materialize =
            config["transforms"][it.first]["materialize"].value_or<std::string>(
                "");
        for (const auto &item : tomlArr) {
          depends.push_back(item.as_string()->get());
        }

        // now all is extracted.
        tfr = new Transform(it.first.data(), depends,
                            std::vector<std::string>{}, sql);
        if (tfr != nullptr) {
          if (materialize == "final") {
            tfr->tfm = TransformMaterialize::FINAL;
            tfr->materialize_table =
                config["transforms"][it.first]["table_name"]
                    .value_or<std::string>("");
          }
          else if (materialize == "parquet"){
            tfr->tfm = TransformMaterialize::PQET;
            tfr->materialize_table = it.first.data();
          }
          else {
            tfr->tfm = TransformMaterialize::TABLE;
            tfr->materialize_table = it.first.data();
          }
          dag->add_node(it.first.data(), tfr);
        }
      }
    }
  }

  dag->attach();
  dag->display();
}