#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"
#include <sstream>

namespace fakeg {
namespace parsers {

class XtbParser : public ParserInterface {
public:
    XtbParser();
    
    // 必需的接口方法
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    bool validateInput(const std::string& filename) override;
    
    std::string getParserName() const override;
    std::string getParserVersion() const override;
    std::vector<std::string> getSupportedKeywords() const override;

private:
    // 解析方法
    bool parseStandardOrientation(std::ifstream& file, data::ParsedData& data);
    bool parseFrequencies(std::ifstream& file, data::ParsedData& data);
    
    // 辅助方法
    bool parseFrequencyBlock(std::ifstream& file, std::vector<data::FreqMode>& frequencies, 
                           const std::vector<int>& freqIndices);
    void parseAtomDisplacements(const std::string& line, int atomIndex, 
                              std::vector<data::FreqMode>& frequencies, 
                              const std::vector<int>& freqIndices);
    
    // 检测标志
    bool xtbFormatDetected;
};

} // namespace parsers
} // namespace fakeg 