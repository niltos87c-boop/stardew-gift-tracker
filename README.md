# 🌾 Stardew Valley Gift Tracker

A lightweight, modular **C command-line tool** that helps you keep track of weekly gifts in *Stardew Valley*. Never waste a Loved Gift again, and never exceed the **2 gifts per villager per week** limit.

![Language](https://img.shields.io/badge/language-C99-blue)
![Build](https://img.shields.io/badge/build-make-green)
![Memory](https://img.shields.io/badge/valgrind%2FASan-leak--free-brightgreen)

## ✨ Features

- Built-in database of villagers and their **Loved Gifts** (Abigail, Sebastian, Haley, Leah, Emily, Shane, Elliott, Sam)
- Tracks gifts given per villager each week (**max 2**)
- Persistent weekly state saved between runs in a plain-text file
- Case-insensitive input (`abigail` == `Abigail`)
- Friendly warnings when a gift is not on a villager's loved list
- Comprehensive error handling with proper exit codes

## 🧠 Technical Highlights

| Topic | Implementation |
|---|---|
| Data modeling | `struct Villager` and `struct VillagerDB` |
| Dynamic memory | Growable arrays via `malloc` / `realloc` / `free` (capacity doubling) |
| Memory safety | `realloc` results are never assigned blindly; verified leak-free with AddressSanitizer / Valgrind |
| CLI | Argument parsing through `argc` / `argv` |
| Architecture | Separate modules: model (`villager`), persistence (`storage`), CLI (`main`) |

## 📁 Project Structure

```
stardew-gift-tracker/
├── Makefile
├── README.md
└── src/
    ├── main.c        # CLI parsing and output
    ├── villager.c    # Data model + in-memory database
    ├── villager.h
    ├── storage.c     # Save/load weekly counters
    └── storage.h
```

## 🔧 Build

Requirements: `gcc` (or any C99 compiler) and `make`.

```bash
make            # optimized build -> ./tracker
make debug      # build with AddressSanitizer + UBSan
make clean      # remove binaries and save file
```

## 🚀 Usage

```bash
./tracker --gift "Abigail" "Amethyst"   # record a gift
./tracker --status                      # weekly overview
./tracker --loved "Haley"               # list loved gifts
./tracker --reset                       # start a new week
./tracker --help
```

### Example session

```text
$ ./tracker --gift "Abigail" "Amethyst"
Gave "Amethyst" to Abigail. They LOVE it!
Gifts remaining this week for Abigail: 1

$ ./tracker --gift "Abigail" "Pumpkin"
Gave "Pumpkin" to Abigail. They LOVE it!
Gifts remaining this week for Abigail: 0

$ ./tracker --gift "Abigail" "Pizza"
Error: weekly gift limit (2) already reached for Abigail.

$ ./tracker --status

=== Weekly Gift Status ===
Villager     Gifts    Progress
-----------------------------------
Abigail      2/2      [##] FULL
Sebastian    0/2      [..]
Haley        0/2      [..]
...
```

### Error handling

- Unknown villager → `Error: unknown villager "Bob".`
- Weekly limit exceeded → rejected, state is not modified
- Missing/extra arguments or unknown options → usage hint, non-zero exit code
- Allocation and file I/O failures are detected and reported

### Save file

Weekly counters are stored in `tracker_data.txt` in the current directory. Override the location with:

```bash
export STARDEW_TRACKER_FILE=~/.stardew_tracker
```

## 🧪 Memory Check

```bash
make debug && ./tracker --status        # ASan/LSan reports nothing
# or
valgrind --leak-check=full ./tracker --status
```

## 🗺️ Roadmap Ideas

- Load villagers from an external JSON/CSV file
- Add Liked / Neutral / Hated gift categories
- Birthday and gift-taste bonuses
- Unit tests with a lightweight framework


