#include "energy_extractors.h"

#include <stdexcept>

namespace fakeg {
namespace parsers {
namespace xyz {

std::string toString(EnergyFormat format) {
    switch (format) {
        case EnergyFormat::Orca:
            return "ORCA";
        case EnergyFormat::Molclus:
            return "molclus";
        case EnergyFormat::Xtb:
            return "xtb";
        default:
            return "unknown";
    }
}

RegexEnergyExtractor::RegexEnergyExtractor(EnergyFormat fmt, std::regex pattern)
    : fmt_(fmt), pattern_(std::move(pattern)) {}

std::optional<double> RegexEnergyExtractor::tryExtract(const std::string& comment) const {
    std::smatch match;
    if (!std::regex_search(comment, match, pattern_)) {
        return std::nullopt;
    }

    if (match.size() < 2) {
        return std::nullopt;
    }

    try {
        const double energy = std::stod(match[1].str());
        return energy;
    } catch (const std::exception&) {
        return std::nullopt;
    }
}

EnergyFormat RegexEnergyExtractor::format() const {
    return fmt_;
}

EnergyExtractorPipeline::EnergyExtractorPipeline(LogFn infoLog, LogFn debugLog)
    : infoLog_(std::move(infoLog)), debugLog_(std::move(debugLog)) {}

void EnergyExtractorPipeline::add(std::unique_ptr<IEnergyExtractor> extractor) {
    extractors_.push_back(std::move(extractor));
}

void EnergyExtractorPipeline::reset() {
    announcedFormats_.clear();
}

void EnergyExtractorPipeline::announceOnce(EnergyFormat format) {
    const int key = static_cast<int>(format);
    if (announcedFormats_.find(key) != announcedFormats_.end()) {
        return;
    }

    announcedFormats_.insert(key);
    if (infoLog_) {
        infoLog_(">> Detected " + toString(format) + " output format - energy information available");
    }
}

std::optional<double> EnergyExtractorPipeline::extract(const std::string& comment) {
    for (const auto& extractor : extractors_) {
        if (!extractor) continue;

        auto energy = extractor->tryExtract(comment);
        if (!energy.has_value()) {
            continue;
        }

        const auto fmt = extractor->format();
        announceOnce(fmt);

        if (debugLog_) {
            debugLog_("Extracted " + toString(fmt) + " energy: " + std::to_string(*energy));
        }

        return energy;
    }

    return std::nullopt;
}

EnergyExtractorPipeline makeDefaultEnergyPipeline(EnergyExtractorPipeline::LogFn infoLog,
                                                 EnergyExtractorPipeline::LogFn debugLog) {
    EnergyExtractorPipeline pipeline(std::move(infoLog), std::move(debugLog));

    // ORCA format:
    // Coordinates from ORCA-job ... E -687.545427056709
    pipeline.add(std::make_unique<RegexEnergyExtractor>(
        EnergyFormat::Orca,
        std::regex(
            R"(Coordinates\s+from\s+ORCA-job\s+.+\s+E\s+([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))")));

    // molclus format:
    // Energy =   -147.48410656 a.u.  #Cluster:    1
    pipeline.add(std::make_unique<RegexEnergyExtractor>(
        EnergyFormat::Molclus,
        std::regex(R"(Energy\s*=\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*a\.u\.)")));

    // xtb format:
    // energy: -149.706157544781 gnorm: 0.499...
    pipeline.add(std::make_unique<RegexEnergyExtractor>(
        EnergyFormat::Xtb,
        std::regex(R"(energy:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))")));

    return pipeline;
}

} // namespace xyz
} // namespace parsers
} // namespace fakeg
