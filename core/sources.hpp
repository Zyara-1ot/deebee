#pragma once
#include "node.hpp"
#include "source_file_type.hpp"


class Sources : public Node{
    public:
        SourceFileType sfile;
        
        //only csv supported till now
        std::string csv_path;
        std::string csv_delim;
        bool csv_header;
        
        int n_file;//no. of files
        
        //need no input we do for sources
        void apply_node() override;

        Sources(const std::string dat,const std::vector<std::string>& _succ,const SourceFileType s,std::string path);
};