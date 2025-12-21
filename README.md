# DeeBee

<a href="https://winter-of-open-source.vercel.app/"><img src="assets/banner.png"></a>

[![C++](https://img.shields.io/badge/C++-%2300599C.svg?logo=c%2B%2B&logoColor=white)](#) [![TOML](https://img.shields.io/badge/TOML-9C4121?logo=toml&logoColor=fff)](#) <a href="https://gdg.community.dev/gdg-on-campus-indian-institute-of-engineering-science-and-technology-shibpur-howrah-india/"><img src="assets/gdsc-logo.png" alt="GDGoC IIEST Shibpur" height="20"></a> <a href="https://www.codeiiest.in/"><img src="assets/codeiiest-logo.png" alt="CodeIIEST" height="20"></a> [![Winter of Open Source](https://img.shields.io/badge/Winter%20of%20Open%20Source-2025-purple)](https://winter-of-open-source.vercel.app/)

## Project Description

**DeeBee** is a single threaded toml based data pipeline creation framework/tool.

This project is part of **IIEST, Shibpur's** **[Winter of Open Source](https://winter-of-open-source.vercel.app/)**.
### What You'll Build

| Component | Current Status | What Needs To Be Done |
|-----------|---------------|----------------------|
| **Core components** | 🟡 Partial | Add support for other file types and events as ingestion, patch security issues |
| **CLI client** | 🔴 Placeholder only | Implement a proper CLI client using core library provided. |

---

## Contribution Workflow

```
1. Fork the repository
2. Clone your fork: git clone https://github.com/YOUR_NAME/deebee.git
3. Create a new branch: git checkout -b fix/issue-number-description
4. Make changes and test thoroughly
5. Commit with proper message: git commit -m "Fixes #<issue-number>: description"
6. Push & create PR: git push origin fix/issue-number-description
```

> See [CONTRIBUTING.md](CONTRIBUTING.md) for detailed setup instructions and guidelines.


## Communication

- **Discord**: [Winter of Open Source Server](https://discord.gg/your-invite-link)
- **Issues**: Comment on issues to reach maintainers

## Build (On Linux)

```bash
git clone https://github.com/sinpea/deebee.git
cd deebee

#build with cmake
mkdir builds
cd builds 
cmake ..
make

#run built executable
./deebee
```

## Build (On Windows)

Similar steps as shown above, use powershell. Also, replace the `libduckdb-linux-amd64` folder with the required files for windows.
For more information: [libduckdb installation](https://duckdb.org/install/?platform=windows&environment=c)

Also, make the following change within `CMakeLists.txt`

```
# -----------------------
# Include directories
# -----------------------
target_include_directories(deebee PRIVATE
    core
    parser
    libduckdb-windows-amd64
)

# -----------------------
# DuckDB library
# -----------------------
target_link_directories(deebee PRIVATE
    ${CMAKE_SOURCE_DIR}/libduckdb-windows-amd64
)

target_link_libraries(deebee PRIVATE
    duckdb
)

# -----------------------
# RPATH (so .so is found at runtime)
# -----------------------
set_target_properties(deebee PROPERTIES
    BUILD_RPATH "${CMAKE_SOURCE_DIR}/libduckdb-windows-amd64"
)

```
## Project Structure

```
.
├── build.sh
├── cli_client
│   └── main.cpp
├── CMakeLists.txt
├── config.toml
├── CONTRIBUTING.md
├── core
│   ├── dependancy_graph.cpp
│   ├── dependancy_graph.hpp
│   ├── duck.cpp
│   ├── duck.hpp
│   ├── meta.cpp
│   ├── meta.hpp
│   ├── node.cpp
│   ├── node.hpp
│   ├── node_type.hpp
│   ├── params.hpp
│   ├── source_file_type.hpp
│   ├── sources.cpp
│   ├── sources.hpp
│   ├── transform_materialize.hpp
│   ├── transforms.cpp
│   └── transforms.hpp
├── data
│   ├── sales.csv
│   └── users.csv
├── libduckdb-linux-amd64
│   ├── duckdb.h
│   ├── duckdb.hpp
│   ├── libduckdb.so
│   └── libduckdb_static.a
├── parser
│   ├── toml.hpp
│   ├── toml_parse.cpp
│   └── toml_parse.hpp
└── README.md
```

## Code of Conduct

Please follow [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md) to ensure a welcoming and productive environment for all contributors.

---

<p align="center">
Made with ❤️ by <a href="https://github.com/sinpea">sinpea</a>
</p>