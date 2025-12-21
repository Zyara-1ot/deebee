#pragma once

#include <variant>
#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>

using PARAM = std::variant<
    std::nullptr_t,
    int64_t,
    double,
    bool,
    std::string
>;

using PARAMS = std::vector<PARAM>;
