#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"

namespace fakeg {
namespace parsers {

class AmespParser : public ParserInterface {
public:
    AmespParser();
    
    // 实现接口方法
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    bool validateInput(const std::string& filename) override;
    
    std::string getParserName() const override;
    std::string getParserVersion() const override;
    std::vector<std::string> getSupportedKeywords() const override;

private:
    // 解析主要方法
    bool parseOptimizationSteps(std::ifstream& file, data::ParsedData& data);
    bool parseSinglePoint(std::ifstream& file, data::ParsedData& data);
    bool parseFrequencies(std::ifstream& file, data::ParsedData& data);
    bool parseThermoData(std::ifstream& file, data::ParsedData& data);
    bool parseTDDFT(std::ifstream& file, data::ParsedData& data);
    
    // 解析辅助方法
    void parseGeometry(std::ifstream& file, std::vector<data::Atom>& atoms);
    double parseEnergyFromCurrentPosition(std::ifstream& file);
    void parseConvergence(std::ifstream& file, data::OptStep& step);
    void parseNormalModes(std::ifstream& file, data::ParsedData& data);
    data::TDDFTData parseTDDFTSection(std::ifstream& file, double eExcValue);
    data::ExcitedState parseExcitedState(std::ifstream& file, const std::string& stateLine, double eExcValue);
    
    // 查找辅助方法
    bool findOptimizationSection(std::ifstream& file);
    bool findFrequencySection(std::ifstream& file);
    bool findThermoSection(std::ifstream& file);
};

} // namespace parsers
} // namespace fakeg 