#pragma once

#include <functional>
#include <optional>
#include <string>

#include "data/structures.h"
#include "parsers/xyz/energy_extractors.h"

namespace fakeg {
namespace parsers {
namespace xyz {

// Parses XYZ comment lines for:
// - charge/spin (frame 1 only; if available)
// - energy (via a configurable extractor pipeline)
class XyzCommentParser {
public:
    using LogFn = std::function<void(const std::string&)>;

    XyzCommentParser(LogFn infoLog, LogFn debugLog);

    void reset();

    // Attempts to update charge/spin (when frameNumber==1 and data has no charge/spin).
    // Returns extracted energy (if any).
    std::optional<double> parse(const std::string& comment, data::ParsedData& data, int frameNumber);

private:
    LogFn infoLog_;
    LogFn debugLog_;
    EnergyExtractorPipeline energyPipeline_;

    void tryExtractChargeSpin(const std::string& comment, data::ParsedData& data, int frameNumber);
};

} // namespace xyz
} // namespace parsers
} // namespace fakeg
