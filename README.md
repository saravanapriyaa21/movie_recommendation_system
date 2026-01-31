# Movie Recommendation System (C++)

A **command-line Movie Recommendation System** implemented in **C++**, showcasing recommendation algorithms, CSV-based data processing, and performance-aware programming using real-world datasets.

This project focuses on **systems-level logic and algorithms**, not UI fluff.

---

## Features

* Secure user authentication (username & password validation)
* Robust CSV parsing with malformed-data tolerance
* Multiple recommendation strategies:

  * **Top Rated Movies** using MapReduce-style aggregation
  * **Collaborative Filtering** using Jaccard similarity
  * **Genre-Based Recommendations** based on user history
  * **Surprise Me** exploration for unseen movies
* Graceful handling of edge cases (including users with no watch history)
* Execution-time measurement for each operation
* Clean, menu-driven CLI interface

---

## Project Structure

```
movie_recommendation_system/
│
├── movie_recommendation_system.cpp   # Main source code
├── data/
│   ├── raw/
│   │   └── movies_raw.csv            # Raw movie dataset (unprocessed)
│   └── processed/
│       ├── movies_processed.csv      # Cleaned & structured movie data (used by program)
│       ├── users.csv                 # User credentials
│       └── watch_history.csv         # User watch history
│
├── .gitignore
└── README.md
```

### Dataset Design (Important)

* **Raw data** is kept separate (`data/raw/`) to preserve original, uncleaned input
* **Processed data** (`data/processed/`) is what the application consumes at runtime
* Ratings are read from `movies_processed.csv`; raw data is included for transparency only
* This separation demonstrates **data engineering discipline**, not duplication

---

## Requirements

* C++17 compatible compiler

  * `g++` (Linux / macOS)
  * `clang++` (macOS)
  * `g++ (MinGW-w64)` or `MSVC` (Windows)
* No external libraries or frameworks
* Standard C++ STL only

---

## How to Build and Run (Local Machine)

### 1. Clone the repository

```bash
git clone https://github.com/saravanapriyaa21/movie_recommendation_system.git
cd movie_recommendation_system
```

---

### 2. Compile the program

#### Linux / macOS

```bash
g++ -std=c++17 movie_recommendation_system.cpp -o movie
```

(macOS alternative)

```bash
clang++ -std=c++17 movie_recommendation_system.cpp -o movie
```

#### Windows (MinGW-w64)

```bash
g++ -std=c++17 movie_recommendation_system.cpp -o movie.exe
```

#### Windows (MSVC – Developer Command Prompt)

```bat
cl /std:c++17 movie_recommendation_system.cpp
```

---

### 3. Run the application

#### Linux / macOS

```bash
./movie
```

#### Windows

```bat
movie.exe
```

---

## Usage

1. Enter a valid **username** and **password** from `data/processed/users.csv`
2. Choose an option from the menu:

```
1. Top Rated Movies
2. Watch History Recommendations
3. Genre Based Recommendations
4. Surprise Me
5. Exit
```

3. Results are displayed along with execution time for transparency

---

## Edge Case Handling

* Users with **no watch history** are handled gracefully
* For collaborative or genre-based recommendations, the system displays a clear message:

  * *“No watch history found. Watch more movies to get personalized recommendations.”*
* The **Surprise Me** option remains available to encourage exploration
* The program never crashes or produces empty/ambiguous output

This behavior reflects **defensive programming and real-world user scenarios**.

---

## Technical Highlights

* **MapReduce-style design** for rating aggregation
* **Set-based similarity metrics** for collaborative filtering
* **Deterministic + randomized recommendation strategies**
* Explicit handling of cold-start users
* Clear separation of data, logic, and execution flow

---

## Notes

* Program expects the `data/` directory to be present at the project root
* All datasets are loaded at runtime from CSV files
* Invalid or missing data rows are safely ignored
* No IDE-specific or OS-specific files are required
