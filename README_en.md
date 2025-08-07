# FakeG - Modular Quantum Chemistry Output Converter

## Project Overview

FakeG is a modular C++ project designed to convert various quantum chemistry software output files into fake Gaussian format files. The project features modern C++ design with excellent extensibility and maintainability.

**Current Supported Formats:**
- **AMESP** → AfakeG executable
- **BDF** → BfakeG executable

## Key Features

- **Modular Design**: Clean code organization with well-defined module responsibilities
- **Extensible Architecture**: Easy to add new parsers through interface design
- **Cross-platform**: Supports Linux and Windows (via cross-compilation)
- **Static Linking**: Multiple linking options for maximum compatibility
- **Comprehensive Logging**: Multi-level logging system with debug support
- **Ghost Atom Support**: Handles unknown elements as Bq (ghost atoms)

## Project Structure

```
FakeG/
├── src/                    # Source code directory
│   ├── data/              # Data structure module
│   │   ├── structures.h   # Data structure definitions
│   │   └── structures.cpp # Element mapping and data structures
│   ├── io/                # IO module
│   │   ├── file_reader.h/cpp    # File reading with encoding detection
│   │   └── gaussian_writer.h/cpp # Gaussian format output
│   ├── logger/            # Logging module
│   │   ├── logger.h       # Multi-level logging system
│   │   └── logger.cpp
│   ├── string/            # String utilities module
│   │   ├── string_utils.h # String processing utilities
│   │   └── string_utils.cpp
│   ├── parsers/           # Parser module
│   │   ├── parser_interface.h/cpp  # Parser base interface
│   │   ├── amesp_parser.h/cpp      # AMESP format parser
│   │   └── bdf_parser.h/cpp        # BDF format parser
│   └── main/              # Main program module
│       ├── fake_g_app.h/cpp        # Application framework
│       ├── afake_g.cpp             # AfakeG main program
│       └── bfake_g.cpp             # BfakeG main program
├── config/                # Configuration files
├── build.sh              # Universal build script
├── build_windows.sh      # Windows cross-compilation script
├── CMakeLists.txt        # CMake build configuration
└── README.md             # Project documentation
```

## Building the Project

### System Requirements

- CMake 3.16+
- C++20 compatible compiler (GCC 10+, Clang 12+, MSVC 2019+)
- For Windows cross-compilation: mingw-w64

### Quick Build

```bash
# Default Linux build (dynamic linking)
./build.sh

# Static linking for better portability
./build.sh linux-static

# Full static linking (including glibc)
./build.sh linux-full

# Windows cross-compilation
./build.sh windows

# Clean build files
./build.sh clean

# Show help
./build.sh help
```

### Manual Build

```bash
# Create build directory
mkdir build && cd build

# Configure (various options available)
cmake ..                          # Default dynamic linking
cmake -DSTATIC_LINKING=ON ..      # Partial static linking
cmake -DFULL_STATIC=ON ..         # Full static linking
cmake -DWINDOWS_BUILD=ON ..       # Windows cross-compilation

# Build
make -j$(nproc)
```

## Usage

### AfakeG (AMESP Format Conversion)

```bash
# Interactive mode (no arguments)
./afakeg

# Command line mode
./afakeg input.aop
./afakeg input.aop --debug
./afakeg input.aop -o output.log
./afakeg --help
```

### BfakeG (BDF Format Conversion)

```bash
# Interactive mode
./bfakeg

# Command line mode
./bfakeg input.out
./bfakeg input.out --debug
```

## Writing New Parsers

### Architecture Overview

The FakeG architecture uses **direct parser injection** rather than factory patterns. Each executable is linked with a specific parser, making the design simple and explicit.

### Step-by-Step Guide

#### 1. Create Parser Header File

Create `src/parsers/your_parser.h`:

```cpp
#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"

namespace fakeg {
namespace parsers {

class YourParser : public ParserInterface {
public:
    YourParser();
    
    // Required interface methods
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    bool validateInput(const std::string& filename) override;
    
    std::string getParserName() const override;
    std::string getParserVersion() const override;
    std::vector<std::string> getSupportedKeywords() const override;

private:
    // Your parsing methods
    bool parseOptimizationSteps(std::ifstream& file, data::ParsedData& data);
    bool parseFrequencies(std::ifstream& file, data::ParsedData& data);
    bool parseThermoData(std::ifstream& file, data::ParsedData& data);
    
    // Helper methods
    void parseGeometry(std::ifstream& file, std::vector<data::Atom>& atoms);
    double parseEnergy(const std::string& line);
};

} // namespace parsers
} // namespace fakeg
```

#### 2. Implement Parser Logic

Create `src/parsers/your_parser.cpp`:

```cpp
#include "your_parser.h"
#include "../string/string_utils.h"
#include <sstream>

namespace fakeg {
namespace parsers {

YourParser::YourParser() = default;

bool YourParser::parse(io::FileReader& reader, data::ParsedData& data) {
    std::ifstream& file = reader.getStream();
    
    infoLog("Starting YourFormat file parsing");
    
    // Reset file position
    string_utils::LineProcessor::resetToBeginning(file);
    
    // Check for optimization
    if (string_utils::LineProcessor::findLine(file, "YOUR_OPT_KEYWORD")) {
        data.hasOpt = true;
        infoLog("Found geometry optimization");
        if (!parseOptimizationSteps(file, data)) {
            errorLog("Optimization parsing failed");
            return false;
        }
    }
    
    // Parse frequencies
    if (parseFrequencies(file, data)) {
        data.hasFreq = true;
        infoLog("Frequency parsing completed");
    }
    
    // Parse thermodynamic data
    if (parseThermoData(file, data)) {
        infoLog("Thermodynamic data parsing completed");
    }
    
    return !data.optSteps.empty();
}

bool YourParser::validateInput(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorLog("Cannot open file: " + filename);
        return false;
    }
    // User-specified file is always intended for conversion
    return true;
}

std::string YourParser::getParserName() const {
    return "YourParser";
}

std::string YourParser::getParserVersion() const {
    return "1.0.0";
}

std::vector<std::string> YourParser::getSupportedKeywords() const {
    return {"YOUR_OPT_KEYWORD", "YOUR_FREQ_KEYWORD", "YOUR_THERMO_KEYWORD"};
}

bool YourParser::parseOptimizationSteps(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    std::string line;
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "YOUR_STEP_MARKER")) {
            data::OptStep step;
            step.stepNumber = extractStepNumber(line);
            step.energy = 0.0;
            
            // Parse geometry for this step
            parseGeometry(file, step.atoms);
            
            // Parse energy
            step.energy = parseEnergy(line);
            
            // Parse convergence data
            // ... your convergence parsing logic
            
            if (!step.atoms.empty()) {
                data.optSteps.push_back(step);
                debugLog("Added step " + std::to_string(step.stepNumber) + 
                        " with " + std::to_string(step.atoms.size()) + " atoms");
            }
        }
    }
    
    return !data.optSteps.empty();
}

// Implement other parsing methods...

} // namespace parsers
} // namespace fakeg
```

#### 3. Create Main Executable

Create `src/main/yourfake_g.cpp`:

```cpp
#include <iostream>
#include "fake_g_app.h"
#include "../parsers/your_parser.h"

using namespace fakeg;

int main(int argc, char* argv[]) {
    // Create your parser instance
    auto parser = std::make_unique<parsers::YourParser>();
    
    // Create application with injected parser
    app::FakeGApp app(std::move(parser));
    
    // Set program information
    app.setProgramInfo("YourFakeG", "1.0.0", "Your Name");
    
    // Run the application
    if (app.run(argc, argv)) {
        return 0;
    } else {
        return 1;
    }
}
```

#### 4. Update CMakeLists.txt

Add your parser to `CMakeLists.txt`:

```cmake
# Add your parser library
set(YOUR_PARSER_SOURCES
    src/parsers/your_parser.cpp
)

add_library(your_parser ${YOUR_PARSER_SOURCES})
target_link_libraries(your_parser fakeg_core)

# Add your executable
add_executable(yourfakeg src/main/yourfake_g.cpp)
target_link_libraries(yourfakeg fakeg_core your_parser)

# Add to install targets
install(TARGETS yourfakeg DESTINATION bin)
install(TARGETS your_parser DESTINATION lib)
```

### Key Parsing Concepts

#### Data Structures

The core data structures you'll work with:

```cpp
// Single atom
data::Atom atom;
atom.symbol = "C";
atom.atomicNumber = 6;  // Or 0 for unknown (Bq ghost atom)
atom.x = atom.y = atom.z = 0.0;

// Optimization step
data::OptStep step;
step.stepNumber = 1;
step.energy = -123.456;
step.atoms = {atom1, atom2, ...};
step.converged = true;

// Frequency mode
data::FreqMode mode;
mode.frequency = 1234.56;
mode.irIntensity = 12.34;
mode.irrep = "A1";
mode.displacements = {{0.1, 0.2, 0.3}, ...}; // Per atom [x,y,z]

// Complete parsed data
data::ParsedData data;
data.hasOpt = true;
data.hasFreq = true;
data.optSteps = {step1, step2, ...};
data.frequencies = {mode1, mode2, ...};
data.thermoData = thermoData;
```

#### Useful Utilities

```cpp
// String utilities
using namespace fakeg::string_utils;

std::string trimmed = trim("  text  ");
std::vector<std::string> tokens = split("a,b,c", ',');
double value = toDouble("123.45", 0.0);
bool isNum = isValidNumber("1.23E-4");

// File line processing
LineProcessor::resetToBeginning(file);
bool found = LineProcessor::findLine(file, "SEARCH_PATTERN");
auto pos = LineProcessor::getPosition(file);
LineProcessor::setPosition(file, pos);

// Element mapping
int atomicNum = elementMap->getAtomicNumber("C");   // Returns 6
int unknown = elementMap->getAtomicNumber("Xyz");   // Returns 0 (Bq ghost)

// Logging
debugLog("Debug information");
infoLog("Progress information");
errorLog("Error occurred");
```

#### Common Parsing Patterns

```cpp
// Pattern 1: Line-by-line parsing
std::string line;
while (std::getline(file, line)) {
    line = string_utils::trim(line);
    
    if (string_utils::contains(line, "ENERGY")) {
        double energy = extractEnergyFromLine(line);
        // ...
    }
}

// Pattern 2: Block parsing
if (string_utils::LineProcessor::findLine(file, "BEGIN_BLOCK")) {
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "END_BLOCK")) break;
        // Process block content
    }
}

// Pattern 3: Structured data parsing
std::istringstream iss(line);
std::string element;
double x, y, z;
if (iss >> element >> x >> y >> z) {
    // Successfully parsed atom coordinates
}
```

### Testing Your Parser

1. **Create test files** in your format
2. **Build your executable**:
   ```bash
   ./build.sh  # or manual cmake/make
   ```
3. **Test with debug output**:
   ```bash
   ./yourfakeg test_file.ext --debug
   ```
4. **Verify output format** matches Gaussian expectations

### Best Practices

1. **Error Handling**: Always check file operations and parsing results
2. **Logging**: Use appropriate log levels (debug/info/error)
3. **Ghost Atoms**: Unknown elements automatically become Bq (atomic number 0)
4. **File Position**: Reset file position when switching between parsing sections
5. **Memory Efficiency**: Parse incrementally, don't load entire file into memory
6. **Validation**: Implement basic input validation in `validateInput()`

## Core Modules Reference

### Logger Module
```cpp
#include "logger/logger.h"

// Available through parser base class
debugLog("Detailed debug information");
infoLog("Progress updates");
errorLog("Error messages");
```

### String Utilities
```cpp
#include "string/string_utils.h"

// Text processing
trim(), ltrim(), rtrim()
split(), toLowerCase(), toUpperCase()
startsWith(), endsWith(), contains()
replace(), replaceAll()

// Number conversion
toDouble(), toInt(), isValidNumber()
removeQuotes()  // Removes surrounding quotes

// File line processing
LineProcessor::findLine()
LineProcessor::resetToBeginning()
```

### Data Structures
All parsers work with the same standardized data structures defined in `data/structures.h`, ensuring compatibility with the Gaussian output writer.

---

## Contact

Author: [Bane Dysta](https://bane-dysta.top/)