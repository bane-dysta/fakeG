#pragma once

#include <string>
#include <fstream>
#include "../data/structures.h"

namespace fakeg {
namespace io {

// Gaussian输出写入器类
class GaussianWriter {
private:
    std::string outputFilename;
    std::string programInfo;
    std::string authorInfo;
    std::string versionInfo;
    
    // 内部写入方法
    void writeHeader(std::ofstream& out, const data::ParsedData& data);
    void writeOptimizationStep(std::ofstream& out, const data::OptStep& step);
    void writeFrequencies(std::ofstream& out, const data::ParsedData& data);
    void writeFrequencyBlock(std::ofstream& out, const data::ParsedData& data, int startIdx, int endIdx);
    void writeThermoData(std::ofstream& out, const data::ThermoData& thermoData);
    void writeConvergenceData(std::ofstream& out, const data::ThermoData& thermoData);
    void writeFooter(std::ofstream& out);
    
    // 格式化辅助方法
    std::string formatEnergy(double energy, int precision = 9);
    std::string formatCoordinate(double coord, int precision = 6);
    std::string formatFrequency(double freq, int precision = 4);
    std::string formatIntensity(double intensity, int precision = 4);
    
public:
    GaussianWriter();
    GaussianWriter(const std::string& outputFilename);
    
    // 设置输出文件
    void setOutputFilename(const std::string& filename);
    std::string getOutputFilename() const;
    
    // 设置程序信息
    void setProgramInfo(const std::string& program, const std::string& version, const std::string& author);
    
    // 主要写入方法
    bool writeGaussianOutput(const data::ParsedData& data);
    bool writeGaussianOutput(const data::ParsedData& data, const std::string& filename);
    
    // 生成输出文件名（根据输入文件名）
    static std::string generateOutputFilename(const std::string& inputFilename, 
                                             const std::string& suffix = "_fake");
    
    // 验证输出是否正确
    bool validateOutput(const std::string& filename);
};

} // namespace io
} // namespace fakeg 