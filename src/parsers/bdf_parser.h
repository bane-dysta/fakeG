#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"

namespace fakeg {
namespace parsers {

class BdfParser : public ParserInterface {
public:
    BdfParser();
    
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
    
    // 解析辅助方法
    void parseGeometryStep(std::ifstream& file, data::OptStep& step);
    void parseConvergence(std::ifstream& file, data::OptStep& step);
    void parseFrequencyBlock(std::ifstream& file, int nFreqs, data::ParsedData& data);
    void parseAtomDisplacements(const std::string& line, int startIdx, int nFreqs, data::ParsedData& data);
    
    // 工具方法
    int countFrequenciesInLine(const std::string& line);
    std::vector<double> parseValuesFromLine(const std::string& line, int nVals);
    
    // 查找辅助方法
    bool findOptimizationSection(std::ifstream& file);
    bool findFrequencySection(std::ifstream& file);
    bool findThermoSection(std::ifstream& file);
};

} // namespace parsers
} // namespace fakeg 