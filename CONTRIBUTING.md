# Contributing to DeeBee

<a href="https://winter-of-open-source.vercel.app/"><img src="assets/banner.png"></a>

Welcome to Winter of Open Source! 🎉    
We're excited to have you contribute to our data pipeline creation framework.

---

> [!IMPORTANT]
> The issues are open for current **IIEST, Shibpur** students only.

## Table of Contents

- [Setting Up Local Environment](#setting-up-local-environment)
- [How to Contribute](#how-to-contribute)
- [How to Write a Clean PR](#how-to-write-a-clean-pr)
- [Scoring Rules](#scoring-rules)
- [Learning Resources](#learning-resources)
- [Code Style](#code-style)
- [Getting Help](#getting-help)

---

## Setting Up Local Environment

### Prerequisites
1. Basic knowledge of C++,SQL
2. Basic understanding of Object Oriented Programming paradigms
3. Git installed on your system
4. A GitHub account

### Installation

1. **Fork the repository**
   
   Click the "Fork" button at the top right of this repository.

2. **Clone your fork**

    ```bash
    git clone https://github.com/YOUR_NAME/deebee.git
    cd deebee
    ```

3. **Build using Make**

   Make sure, `CMake` is installed on your system.

   ```bash
   cd deebee
   mkdir builds
   cd builds
   cmake ..
   make
   ```

4. **Run using** `./deebee <working_directory_with_config.toml>`

### Dependencies

- `toml++`
- `libduckdb`

---

## How to Contribute

### Step 1: Choose an Issue

* Browse [open issues](../../issues)
* Look for labels:
  * `good first issue` / `beginner-friendly` – great for newcomers
  * `easy`, `medium`, `hard` – based on difficulty
  * `documentation`, `bug`, `feature`, `enhancement`

### Step 2: Comment `/assign`

* Comment `/assign` on the issue you want to work on
* Wait for maintainer approval
* **Only 1 person per issue at a time**
* **Complete within 72 hours** or it gets unassigned

### Step 3: Create a Branch

```bash
git checkout -b fix/issue-number-short-description
```

Example branch names:
* `fix/23-add-error-correction`
* `feature/46-implement-python-transforms`
* `docs/12-improve-readme`

### Step 4: Make Changes

* Follow existing code style.
* Add meaningful comments explaining **why**, not just what.
* Test your changes thoroughly using different sets of inputs, at least verify using provided `config.toml` and sample data. (Testing framework to be added soon)
* Keep commits atomic.

### Step 5: Commit Changes

```bash
git add .
git commit -m "Fixes #23: Fixed incorrect parsing of path in CLI client

- Implemented filters to ensure valid path
- Implemented checks for existence of path"
```

### Step 6: Push & Create PR

```bash
git push origin fix/issue-number-short-description
```

* Go to your fork on GitHub → "Compare & pull request"
* Fill out the PR template completely
* Link the issue using `Fixes #<issue-number>`

---

## How to Write a Clean PR

### Must Include:
- Link to the issue: `Fixes #<issue-number>`
- Clear description of what you changed
- Tested builds with testing frameworks (once provided), or with provided sample config.toml

### Code Requirements:
- Proper indentation
- Meaningful comments
- No console errors
- Adhering to current structure of Classes and wrappers.

### PR Template :
- Open `.github/pull_request_template.md` for guidance
- Moreover, for other templates refer to the `.github/ISSUE_TEMPLATE/` folder
  
## Scoring Rules

## Issue Labels

| Label | Description |
|-------|-------------|
| `easy` | Beginner-friendly, small fixes |
| `medium` | Moderate complexity, features |
| `hard` | Complex tasks, major features |
| `documentation` | Documentation improvements |
| `bug` | Something isn't working |
| `feature` | New feature request |
| `good-first-issue` | Great for newcomers |
| `beginner-friendly` | Suitable for beginners |

### Points Per PR

| PR Type | Points |
|---------|--------|
| **Easy** | 10 |
| **Medium** | 20 |
| **Hard** | 40 |

### Bonuses

| Bonus | Points |
|-------|--------|
|**First 10 PRs** | +10 |
| **First PR of the Week** (resets Monday 12 AM) | +10 |
| **Most Impactful PR** (decided at end) | +50 |
|**New issue added** | +20 |

### Rules

- **Only merged PRs count**
- Work on **only 1 issue at a time**
- Complete within **72 hours** or issue gets unassigned
- Moreover, you can **raise your own issues**, those will be added if impactful for the repo

---

## Learning Resources

Before you start contributing, we **strongly recommend** learning the fundamentals of OOPs and how databases work. The goal is to convert this **mvp** into a finished, secure, robust framework.

### Essential Reading

Follow this comprehensive 7-part tutorial series:

| Part | Title | Topics Covered |
|------|-------|----------------|
| 1 | [Basic Concepts](https://www.learncpp.com/cpp-tutorial/introduction-to-object-oriented-programming/) |Basic concepts of C++, OOP |
| 2 | [Graphs](https://opendsa-server.cs.vt.edu/OpenDSA/Books/DSAF23notes/html/Graphs.html) | Types of Graphs, Traversals, Graph algorithms |
| 3 | [Topological Sorting](https://www.geeksforgeeks.org/cpp/cpp-program-for-topological-sorting/) | Topological Sorting of Discrete Acyclic Graph |
| 4 | [Basics of SQL](https://www.geeksforgeeks.org/sql/sql-tutorial/) | Basic DBMS concepts, SQL |
| 5 | [DuckDB Tutorial](https://motherduck.com/blog/duckdb-tutorial-for-beginners/) | Introduction to DuckDB, using the CLI |
| 6 | [DuckDB resources](https://github.com/davidgasquez/awesome-duckdb) | Resources for DuckDB |
| 7 | [Basics of buildsystems (CMake)](https://www.youtube.com/watch?v=nlKcXPUJGwA) | Basics of build systems, basic usage of CMake |

> [!IMPORTANT]
> **Please avoid making changes to CMakeLists.txt** unless necessary. Also, **make sure that proper abstraction is maintained** to prevent the codebase from becoming unmanagable.

---

## Code Style

- Use proper **indentation**
- Use **descriptive variable names**
- Explain **why**, not just what in comments
- **Logic** for the functions, algorithms used should be clear

---

## Getting Help

- **Discord**: [Winter of Open Source Server](https://discord.gg/EzvckznUDG) (everyone is requested to join this server)
- **GitHub Discussions**: Ask questions, share ideas
- **Issues**: Comment to reach maintainers

---

## Important Rules

- Work on **one issue at a time**
- Complete within **72 hours** (can be extended based on difficulty)
- Respect the code of conduct
- Always link your PR to an issue
- **No plagiarism**
- Keeep **AI** use **minimal** and **relevant**, i.e. only for assistance, not for entire code
---

<p align="center">
<b>Happy Contributing! ❤️</b>
</p>