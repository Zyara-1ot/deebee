#include "../core/dependancy_graph.hpp"
#include "toml.hpp"

#include "../core/node.hpp"

class TomlParse {
public:
  std::string work_dir;

  toml::table config;
  toml::node_view<toml::node> settings;
  toml::node_view<toml::node> sources;
  toml::node_view<toml::node> transforms;

  DependancyGraph *dag;
  void run_stuff();

  TomlParse(const std::string& workdir);
};