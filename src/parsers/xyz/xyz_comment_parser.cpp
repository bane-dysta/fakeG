#include "xyz_comment_parser.h"

#include <sstream>
#include <vector>

#include "string/string_utils.h"

namespace fakeg {
namespace parsers {
namespace xyz {

XyzCommentParser::XyzCommentParser(LogFn infoLog, LogFn debugLog)
    : infoLog_(std::move(infoLog)),
      debugLog_(std::move(debugLog)),
      energyPipeline_(makeDefaultEnergyPipeline(infoLog_, debugLog_)) {}

void XyzCommentParser::reset() {
    energyPipeline_.reset();
}

void XyzCommentParser::tryExtractChargeSpin(const std::string& comment, data::ParsedData& data, int frameNumber) {
    // Only attempt in frame 1, and only if not already present.
    if (frameNumber != 1 || data.hasChargeSpinInfo) {
        return;
    }

    const std::string trimmed = string_utils::trim(comment);

    // Tokenize by whitespace.
    std::istringstream iss(trimmed);
    std::vector<std::string> tokens;
    std::string token;

    while (iss >> token) {
        tokens.push_back(token);
    }

    // Expect exactly 2 integer tokens.
    if (tokens.size() != 2) {
        return;
    }

    const bool isFirstInt = string_utils::isValidNumber(tokens[0]) && tokens[0].find('.') == std::string::npos;
    const bool isSecondInt = string_utils::isValidNumber(tokens[1]) && tokens[1].find('.') == std::string::npos;

    if (!isFirstInt || !isSecondInt) {
        return;
    }

    try {
        data.charge = string_utils::toInt(tokens[0], 0);
        data.spin = string_utils::toInt(tokens[1], 1);
        data.hasChargeSpinInfo = true;

        if (infoLog_) {
            infoLog_("Extracted charge: " + std::to_string(data.charge) + ", spin: " + std::to_string(data.spin) +
                     " from first frame");
        }
    } catch (const std::exception& e) {
        if (debugLog_) {
            debugLog_("Failed to parse charge/spin: " + std::string(e.what()));
        }
    }
}

std::optional<double> XyzCommentParser::parse(const std::string& comment, data::ParsedData& data, int frameNumber) {
    tryExtractChargeSpin(comment, data, frameNumber);
    return energyPipeline_.extract(comment);
}

} // namespace xyz
} // namespace parsers
} // namespace fakeg
