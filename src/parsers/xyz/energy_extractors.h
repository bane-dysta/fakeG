#pragma once

#include <functional>
#include <memory>
#include <optional>
#include <regex>
#include <string>
#include <unordered_set>
#include <vector>

namespace fakeg {
namespace parsers {
namespace xyz {

enum class EnergyFormat {
    Orca,
    Molclus,
    Xtb,
};

std::string toString(EnergyFormat format);

class IEnergyExtractor {
public:
    virtual ~IEnergyExtractor() = default;

    // Returns energy if this extractor recognizes the format.
    virtual std::optional<double> tryExtract(const std::string& comment) const = 0;

    virtual EnergyFormat format() const = 0;
};

// A regex-based extractor that captures the energy in capture group 1.
class RegexEnergyExtractor final : public IEnergyExtractor {
public:
    RegexEnergyExtractor(EnergyFormat fmt, std::regex pattern);

    std::optional<double> tryExtract(const std::string& comment) const override;
    EnergyFormat format() const override;

private:
    EnergyFormat fmt_;
    std::regex pattern_;
};

// Stateful pipeline that tries multiple extractors in order and
// logs a one-time "Detected ..." message per format.
class EnergyExtractorPipeline {
public:
    using LogFn = std::function<void(const std::string&)>;

    EnergyExtractorPipeline(LogFn infoLog, LogFn debugLog);

    void add(std::unique_ptr<IEnergyExtractor> extractor);

    // Reset one-time detection state.
    void reset();

    // Try to extract energy from a comment line.
    std::optional<double> extract(const std::string& comment);

private:
    LogFn infoLog_;
    LogFn debugLog_;
    std::vector<std::unique_ptr<IEnergyExtractor>> extractors_;
    std::unordered_set<int> announcedFormats_;

    void announceOnce(EnergyFormat format);
};

// Factory for the default pipeline used by XyzParser.
EnergyExtractorPipeline makeDefaultEnergyPipeline(EnergyExtractorPipeline::LogFn infoLog,
                                                 EnergyExtractorPipeline::LogFn debugLog);

} // namespace xyz
} // namespace parsers
} // namespace fakeg
