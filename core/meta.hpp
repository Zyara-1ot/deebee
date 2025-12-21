#pragma once
#include <string>

struct Meta {
    std::string work_dir;
    std::string project_name;
};

Meta& meta();  // accessor