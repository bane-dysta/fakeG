#pragma once

#include "parser_interface.h"
#include "../string/string_utils.h"
#include <regex>

namespace fakeg {
namespace parsers {

class XyzParser : public ParserInterface {
public:
    XyzParser();
    
    // 必需的接口方法
    bool parse(io::FileReader& reader, data::ParsedData& data) override;
    bool validateInput(const std::string& filename) override;
    
    std::string getParserName() const override;
    std::string getParserVersion() const override;
    std::vector<std::string> getSupportedKeywords() const override;

private:
    // XYZ解析方法
    bool parseXyzTrajectory(std::ifstream& file, data::ParsedData& data);
    bool parseXyzFrame(std::ifstream& file, data::OptStep& step, int frameNumber);
    
    // 能量提取方法
    double extractEnergyFromComment(const std::string& comment, bool& energyFound);
    double extractMolclusEnergy(const std::string& comment);
    double extractXtbEnergy(const std::string& comment);
    
    // 辅助方法
    void parseAtomLine(const std::string& line, data::Atom& atom);
    
    // 统计信息
    int totalFrames;
    int framesWithEnergy;
    
    // 格式提示标志
    bool molclusDetected;
    bool xtbDetected;
};

} // namespace parsers
} // namespace fakeg 