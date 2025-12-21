#include <iostream>
#include <filesystem>
#include "../parser/toml_parse.hpp"

int main(int argc,char *argv[]){
    if(argc < 2){
        std::cout << "Usage: deebee <working_directory>" << "\n";
        return 0;
    }

    std::string working_dir = std::filesystem::current_path().string() + "/" + argv[1];

    TomlParse tom = TomlParse(working_dir);
    tom.run_stuff();
}