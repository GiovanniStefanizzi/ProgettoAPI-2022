## 🧠 WordChecker — Final Project in Algorithms and Data Structures

This project is a final assignment for the "Algorithms and Principles of Computer Science" course, focused on applying algorithmic techniques and data structures in a practical context using the C programming language.

### 🚀 Project Overview

**WordChecker** is a console-based tool that compares two equal-length strings (called "words") and generates a result string that encodes their similarity. The comparison follows specific rules based on exact matches, partial matches, and character frequencies. The tool simulates a word-guessing game with additional features such as:

- Multiple game sessions (`+nuova_partita`)
- Word filtering based on learned constraints
- Dynamic addition of new valid words during execution
- Efficient matching logic using positional and frequency-based constraints

### ⚙️ Technical Details

- Language: **C (C11)**
- Input: from `stdin`
- Output: to `stdout`
- No external libraries or multithreading used
- Fully self-contained and deterministic

### 📋 Functionality

- Reads a word length `k` and a list of valid words
- Starts a new game session on `+nuova_partita`
- Compares words against a reference word, outputting:
  - `+` for correct letter in the correct position
  - `|` for correct letter in the wrong position
  - `/` for letters not present in the reference word
- Outputs `ok` if a word matches exactly, `ko` if none matched
- Prints `not_exists` if an invalid word is used
- Filters and lists all valid words consistent with learned constraints via `+stampa_filtrate`
