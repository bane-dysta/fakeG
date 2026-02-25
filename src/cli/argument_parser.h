#pragma once

#include <string>
#include <vector>

namespace fakeg {
namespace cli {

// Simple command line argument parser.
//
// Notes:
// - Supports flags (e.g. --debug, -h)
// - Supports key-value options (e.g. -o out.log, --output out.log)
// - Positional args are args that are not options/flags.
class ArgumentParser {
private:
    std::vector<std::string> args_;
    std::string programName_;

public:
    ArgumentParser(int argc, char* argv[]);

    bool hasFlag(const std::string& flag) const;
    std::string getValue(const std::string& option, const std::string& defaultValue = "") const;
    std::string getPositionalArg(size_t index, const std::string& defaultValue = "") const;
    size_t getPositionalArgCount() const;

    std::string getProgramName() const;

    void printUsage(const std::string& description,
                    const std::vector<std::string>& options = {},
                    const std::vector<std::string>& examples = {}) const;
};

} // namespace cli
} // namespace fakeg
