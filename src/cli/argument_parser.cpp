#include "argument_parser.h"

#include <algorithm>
#include <iostream>

namespace fakeg {
namespace cli {

ArgumentParser::ArgumentParser(int argc, char* argv[]) {
    if (argc > 0) {
        programName_ = argv[0];
        // Strip path.
        size_t lastSlash = programName_.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            programName_ = programName_.substr(lastSlash + 1);
        }
    }

    for (int i = 1; i < argc; i++) {
        args_.emplace_back(argv[i]);
    }
}

bool ArgumentParser::hasFlag(const std::string& flag) const {
    return std::find(args_.begin(), args_.end(), flag) != args_.end();
}

std::string ArgumentParser::getValue(const std::string& option, const std::string& defaultValue) const {
    auto it = std::find(args_.begin(), args_.end(), option);
    if (it != args_.end() && (it + 1) != args_.end()) {
        return *(it + 1);
    }
    return defaultValue;
}

std::string ArgumentParser::getPositionalArg(size_t index, const std::string& defaultValue) const {
    std::vector<std::string> positional;

    for (size_t i = 0; i < args_.size(); i++) {
        const std::string& arg = args_[i];

        // Skip options/flags
        if (arg.starts_with("--") || arg.starts_with("-")) {
            // If next token exists and is not another option, treat it as a value and skip it.
            if (i + 1 < args_.size() && !args_[i + 1].starts_with("-")) {
                i++;
            }
            continue;
        }

        positional.push_back(arg);
    }

    if (index < positional.size()) {
        return positional[index];
    }

    return defaultValue;
}

size_t ArgumentParser::getPositionalArgCount() const {
    size_t count = 0;

    for (size_t i = 0; i < args_.size(); i++) {
        const std::string& arg = args_[i];

        if (arg.starts_with("--") || arg.starts_with("-")) {
            if (i + 1 < args_.size() && !args_[i + 1].starts_with("-")) {
                i++;
            }
            continue;
        }

        count++;
    }

    return count;
}

std::string ArgumentParser::getProgramName() const {
    return programName_;
}

void ArgumentParser::printUsage(const std::string& description,
                               const std::vector<std::string>& options,
                               const std::vector<std::string>& examples) const {
    std::cout << "Usage: " << programName_ << " [options] <input_file>" << std::endl;
    std::cout << std::endl;

    if (!description.empty()) {
        std::cout << description << std::endl;
        std::cout << std::endl;
    }

    if (!options.empty()) {
        std::cout << "Options:" << std::endl;
        for (const auto& option : options) {
            std::cout << "  " << option << std::endl;
        }
        std::cout << std::endl;
    }

    if (!examples.empty()) {
        std::cout << "Examples:" << std::endl;
        for (const auto& example : examples) {
            std::cout << "  " << example << std::endl;
        }
        std::cout << std::endl;
    }
}

} // namespace cli
} // namespace fakeg
