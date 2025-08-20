#include "xyz_parser.h"
#include "../string/string_utils.h"
#include <sstream>
#include <regex>
#include <algorithm>
#include <cctype>

namespace fakeg {
namespace parsers {

XyzParser::XyzParser() : totalFrames(0), framesWithEnergy(0), molclusDetected(false), xtbDetected(false) {}

bool XyzParser::parse(io::FileReader& reader, data::ParsedData& data) {
    std::ifstream& file = reader.getStream();
    
    infoLog("Starting XYZ trajectory file parsing");
    
    // 重置文件位置
    string_utils::LineProcessor::resetToBeginning(file);
    
    // 解析XYZ轨迹
    if (parseXyzTrajectory(file, data)) {
        data.hasOpt = true;
        infoLog("XYZ trajectory parsing completed");
        
        if (framesWithEnergy > 0) {
            infoLog("Energy information available in " + std::to_string(framesWithEnergy) + 
                   " out of " + std::to_string(totalFrames) + " frames");
        } else {
            infoLog("No energy information found in comments, using default values");
        }
        
        return true;
    }
    
    errorLog("XYZ trajectory parsing failed");
    return false;
}

bool XyzParser::validateInput(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorLog("Cannot open file: " + filename);
        return false;
    }
    
    // 用户指定的文件就是想要转换的文件，不需要额外验证
    (void)filename; // 抑制未使用参数警告
    return true;
}

std::string XyzParser::getParserName() const {
    return "XyzParser";
}

std::string XyzParser::getParserVersion() const {
    return "1.0.0";
}

std::vector<std::string> XyzParser::getSupportedKeywords() const {
    return {"XYZ", "TRJ", "TRAJECTORY"};
}

bool XyzParser::parseXyzTrajectory(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    totalFrames = 0;
    framesWithEnergy = 0;
    molclusDetected = false;
    xtbDetected = false;
    
    std::string line;
    
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        // 跳过空行
        if (line.empty()) continue;
        
        // 检查是否是原子数行
        if (string_utils::isValidNumber(line)) {
            int numAtoms = string_utils::toInt(line, 0);
            if (numAtoms > 0) {
                data::OptStep step;
                step.stepNumber = totalFrames + 1;
                
                if (parseXyzFrame(file, step, totalFrames + 1, data)) {
                    if (!step.atoms.empty()) {
                        data.optSteps.push_back(step);
                        totalFrames++;
                        debugLog("Added frame " + std::to_string(totalFrames) + 
                                " with " + std::to_string(step.atoms.size()) + " atoms");
                    }
                } else {
                    errorLog("Failed to parse frame " + std::to_string(totalFrames + 1));
                    break;
                }
            }
        }
    }
    
    return totalFrames > 0;
}

bool XyzParser::parseXyzFrame(std::ifstream& file, data::OptStep& step, int frameNumber, data::ParsedData& data) {
    std::string commentLine;
    
    // 读取注释行
    if (!std::getline(file, commentLine)) {
        errorLog("Failed to read comment line for frame " + std::to_string(frameNumber));
        return false;
    }
    
    commentLine = string_utils::trim(commentLine);
    
    // 尝试从注释行提取能量
    bool energyFound = false;
    double energy = extractEnergyFromComment(commentLine, energyFound, data, frameNumber);
    
    if (energyFound) {
        step.energy = energy;
        framesWithEnergy++;
        debugLog("Extracted energy " + std::to_string(energy) + " from frame " + std::to_string(frameNumber));
    } else {
        step.energy = -100.0; // 默认能量值
    }
    
    // 设置收敛状态（XYZ轨迹中没有收敛信息，设为false）
    step.converged = false;
    
    // 读取原子坐标
    std::string atomLine;
    while (std::getline(file, atomLine)) {
        atomLine = string_utils::trim(atomLine);
        
        // 如果遇到空行或数字行（下一帧开始），停止读取当前帧
        if (atomLine.empty()) {
            break;
        }
        
        if (string_utils::isValidNumber(atomLine)) {
            // 回退一行，这是下一帧的原子数
            file.seekg(-static_cast<int>(atomLine.length() + 1), std::ios::cur);
            break;
        }
        
        // 解析原子行
        data::Atom atom;
        parseAtomLine(atomLine, atom);
        
        if (!atom.symbol.empty()) {
            step.atoms.push_back(atom);
        }
    }
    
    return !step.atoms.empty();
}

double XyzParser::extractEnergyFromComment(const std::string& comment, bool& energyFound, data::ParsedData& data, int frameNumber) {
    energyFound = false;
    
    // 在第一帧时尝试提取charge和spin信息
    if (frameNumber == 1 && !data.hasChargeSpinInfo) {
        std::string trimmed = string_utils::trim(comment);
        
        // 将字符串按空格分割
        std::istringstream iss(trimmed);
        std::vector<std::string> tokens;
        std::string token;
        
        while (iss >> token) {
            tokens.push_back(token);
        }
        
        // 检查是否有且仅有两个token，并且都是整数
        if (tokens.size() == 2) {
            try {
                bool isFirstInt = string_utils::isValidNumber(tokens[0]);
                bool isSecondInt = string_utils::isValidNumber(tokens[1]);
                
                if (isFirstInt && isSecondInt) {
                    // 检查是否为整数（不包含小数点）
                    if (tokens[0].find('.') == std::string::npos && 
                        tokens[1].find('.') == std::string::npos) {
                        
                        data.charge = string_utils::toInt(tokens[0], 0);
                        data.spin = string_utils::toInt(tokens[1], 1);
                        data.hasChargeSpinInfo = true;
                        
                        infoLog("Extracted charge: " + std::to_string(data.charge) + 
                               ", spin: " + std::to_string(data.spin) + " from first frame");
                        
                        // 如果第二行只有charge和spin信息，直接返回
                        return 0.0;
                    }
                }
            } catch (const std::exception& e) {
                debugLog("Failed to parse charge/spin: " + std::string(e.what()));
            }
        }
    }
    
    // 尝试molclus格式：Energy =   -147.48410656 a.u.  #Cluster:    1
    double energy = extractMolclusEnergy(comment);
    if (energy != 0.0) {
        energyFound = true;
        return energy;
    }
    
    // 尝试xtb格式：energy: -149.706157544781 gnorm: 0.499531841458 xtb: 6.7.0 (75e6a61)
    energy = extractXtbEnergy(comment);
    if (energy != 0.0) {
        energyFound = true;
        return energy;
    }
    
    return 0.0;
}

double XyzParser::extractMolclusEnergy(const std::string& comment) {
    // 使用正则表达式匹配：Energy = 数字 a.u.
    std::regex molclusPattern(R"(Energy\s*=\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?)\s*a\.u\.)");
    std::smatch match;
    
    if (std::regex_search(comment, match, molclusPattern)) {
        try {
            double energy = std::stod(match[1].str());
            debugLog("Extracted molclus energy: " + std::to_string(energy));
            
            // 只在第一次检测时输出提示
            if (!molclusDetected) {
                infoLog(">> Detected molclus output format - energy information available");
                molclusDetected = true;
            }
            
            return energy;
        } catch (const std::exception& e) {
            debugLog("Failed to convert molclus energy: " + std::string(e.what()));
        }
    }
    
    return 0.0;
}

double XyzParser::extractXtbEnergy(const std::string& comment) {
    // 使用正则表达式匹配：energy: 数字
    std::regex xtbPattern(R"(energy:\s*([-+]?\d*\.?\d+(?:[eE][-+]?\d+)?))");
    std::smatch match;
    
    if (std::regex_search(comment, match, xtbPattern)) {
        try {
            double energy = std::stod(match[1].str());
            debugLog("Extracted xtb energy: " + std::to_string(energy));
            
            // 只在第一次检测时输出提示
            if (!xtbDetected) {
                infoLog(">> Detected xtb output format - energy information available");
                xtbDetected = true;
            }
            
            return energy;
        } catch (const std::exception& e) {
            debugLog("Failed to convert xtb energy: " + std::string(e.what()));
        }
    }
    
    return 0.0;
}

void XyzParser::parseAtomLine(const std::string& line, data::Atom& atom) {
    std::istringstream iss(line);
    std::string symbol;
    double x, y, z;
    
    if (iss >> symbol >> x >> y >> z) {
        atom.symbol = symbol;
        atom.x = x;
        atom.y = y;
        atom.z = z;
        
        // 通过ElementMap获取原子序数
        if (elementMap) {
            atom.atomicNumber = elementMap->getAtomicNumber(symbol);
        } else {
            atom.atomicNumber = 0; // 默认为碳
        }
        
        debugLog("Parsed atom: " + symbol + " (" + std::to_string(atom.atomicNumber) + ") " +
                 std::to_string(x) + " " + std::to_string(y) + " " + std::to_string(z));
    } else {
        errorLog("Failed to parse atom line: " + line);
        atom.symbol = "";
    }
}

} // namespace parsers
} // namespace fakeg 