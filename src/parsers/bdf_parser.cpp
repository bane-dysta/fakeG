#include "bdf_parser.h"
#include "../string/string_utils.h"
#include <sstream>
#include <algorithm>

namespace fakeg {
namespace parsers {

BdfParser::BdfParser() = default;

bool BdfParser::parse(io::FileReader& reader, data::ParsedData& data) {
    std::ifstream& file = reader.getStream();
    
    infoLog("开始解析BDF文件");
    
    // 检查是否是优化计算
    string_utils::LineProcessor::resetToBeginning(file);
    if (findOptimizationSection(file)) {
        data.hasOpt = true;
        infoLog("发现几何优化");
        if (!parseOptimizationSteps(file, data)) {
            errorLog("优化步骤解析失败");
            return false;
        }
        infoLog("总优化步骤: " + std::to_string(data.optSteps.size()));
    } else {
        // 单点计算
        infoLog("单点计算检测");
        if (!parseSinglePoint(file, data)) {
            errorLog("单点计算解析失败");
            return false;
        }
    }
    
    // 解析频率
    if (parseFrequencies(file, data)) {
        data.hasFreq = true;
        infoLog("频率解析完成");
    }
    
    // 解析热力学数据
    if (parseThermoData(file, data)) {
        infoLog("热力学数据解析完成");
    }
    
    return !data.optSteps.empty();
}

bool BdfParser::validateInput(const std::string& filename) {
    // 只检查文件是否存在，不检查扩展名
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorLog("无法打开文件: " + filename);
        return false;
    }
    
    // 用户指定的文件就是想要转换的文件，不需要额外验证
    return true;
}

std::string BdfParser::getParserName() const {
    return "BdfParser";
}

std::string BdfParser::getParserVersion() const {
    return "1.1.0";
}

std::vector<std::string> BdfParser::getSupportedKeywords() const {
    return {"Geometry Optimization step", "Results of vibrations", "Thermal Contributions to Energies", "Atom         Coord"};
}

bool BdfParser::findOptimizationSection(std::ifstream& file) {
    return string_utils::LineProcessor::findLine(file, "Geometry Optimization step");
}

bool BdfParser::findFrequencySection(std::ifstream& file) {
    return string_utils::LineProcessor::findLine(file, "Results of vibrations:");
}

bool BdfParser::findThermoSection(std::ifstream& file) {
    return string_utils::LineProcessor::findLine(file, "Thermal Contributions to Energies");
}

bool BdfParser::parseOptimizationSteps(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    std::string line;
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "Geometry Optimization step :")) {
            data::OptStep step;
            step.energy = 0.0;
            step.rmsGrad = step.maxGrad = step.rmsStep = step.maxStep = 0.0;
            step.converged = false;
            
            // 提取步骤编号
            size_t pos = line.find(":");
            if (pos != std::string::npos) {
                std::string stepStr = string_utils::trim(line.substr(pos + 1));
                step.stepNumber = string_utils::toInt(stepStr, 1);
                debugLog("处理优化步骤 " + std::to_string(step.stepNumber));
            }
            
            // 解析几何
            parseGeometryStep(file, step);
            
            // 解析收敛
            parseConvergence(file, step);
            
            if (!step.atoms.empty()) {
                data.optSteps.push_back(step);
                debugLog("添加步骤 " + std::to_string(step.stepNumber) + "，包含 " + 
                        std::to_string(step.atoms.size()) + " 个原子，能量 = " + std::to_string(step.energy));
            }
        }
    }
    
    return !data.optSteps.empty();
}

bool BdfParser::parseSinglePoint(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    data::OptStep step;
    step.stepNumber = 1;
    step.converged = true;
    
    parseGeometryStep(file, step);
    
    if (!step.atoms.empty()) {
        data.optSteps.push_back(step);
        return true;
    }
    
    return false;
}

void BdfParser::parseGeometryStep(std::ifstream& file, data::OptStep& step) {
    std::string line;
    
    // 查找 "Atom         Coord" 部分
    if (!string_utils::LineProcessor::findLine(file, "Atom         Coord")) {
        debugLog("警告: 步骤 " + std::to_string(step.stepNumber) + " 找不到 Atom Coord 部分");
        return;
    }
    
    // 读取原子直到遇到 State= 或 Energy= 或空行
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        if (line.empty() || string_utils::contains(line, "State=")) {
            break;
        }
        
        if (string_utils::contains(line, "Energy=")) {
            // 从此行解析能量
            size_t pos = line.find("Energy=");
            std::string energyStr = line.substr(pos + 7);
            step.energy = string_utils::toDouble(string_utils::trim(energyStr), 0.0);
            break;
        }
        
        // 解析原子行: Element X Y Z
        std::istringstream iss(line);
        std::string element;
        double x, y, z;
        
        if (iss >> element >> x >> y >> z) {
            data::Atom atom;
            atom.symbol = element;
            atom.atomicNumber = elementMap->getAtomicNumber(element);
            atom.x = x;
            atom.y = y;
            atom.z = z;
            step.atoms.push_back(atom);
            
            debugLog("步骤 " + std::to_string(step.stepNumber) + " - 读取原子: " + element + 
                    " (" + std::to_string(atom.atomicNumber) + ") 位于 (" + 
                    std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
        }
    }
    
    // 如果从坐标部分没有获得能量，继续查找
    if (step.energy == 0.0) {
        while (std::getline(file, line)) {
            if (string_utils::contains(line, "Energy=")) {
                size_t pos = line.find("Energy=");
                std::string energyStr = line.substr(pos + 7);
                step.energy = string_utils::toDouble(string_utils::trim(energyStr), 0.0);
                break;
            }
            // 如果遇到下一个部分就停止
            if (string_utils::contains(line, "Force-RMS") || 
                string_utils::contains(line, "Geometry Optimization step")) {
                break;
            }
        }
    }
}

void BdfParser::parseConvergence(std::ifstream& file, data::OptStep& step) {
    std::string line;
    bool foundConvergence = false;
    
    // 查找当前几何后的收敛值
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "Current values")) {
            // 从同一行或下一行解析值
            std::istringstream iss(line);
            std::string dummy;
            
            // 跳过 "Current values  :"
            iss >> dummy >> dummy >> dummy;
            
            if (iss >> step.rmsGrad >> step.maxGrad >> step.rmsStep >> step.maxStep) {
                foundConvergence = true;
                debugLog("步骤 " + std::to_string(step.stepNumber) + " 收敛: RMS Grad=" + std::to_string(step.rmsGrad) + 
                        ", Max Grad=" + std::to_string(step.maxGrad) + ", RMS Step=" + std::to_string(step.rmsStep) + 
                        ", Max Step=" + std::to_string(step.maxStep));
            } else {
                // 值可能在下一行
                if (std::getline(file, line)) {
                    std::istringstream iss2(line);
                    if (iss2 >> step.rmsGrad >> step.maxGrad >> step.rmsStep >> step.maxStep) {
                        foundConvergence = true;
                        debugLog("步骤 " + std::to_string(step.stepNumber) + " 收敛 (下一行): RMS Grad=" + std::to_string(step.rmsGrad) + 
                                ", Max Grad=" + std::to_string(step.maxGrad) + ", RMS Step=" + std::to_string(step.rmsStep) + 
                                ", Max Step=" + std::to_string(step.maxStep));
                    }
                }
            }
            break;
        }
        
        // 只在遇到下一个优化步骤或频率分析时停止
        if (string_utils::contains(line, "Geometry Optimization step") ||
            string_utils::contains(line, "Results of vibrations") ||
            string_utils::contains(line, "Start analytical Hessian")) {
            break;
        }
    }
    
    if (!foundConvergence) {
        debugLog("警告: 步骤 " + std::to_string(step.stepNumber) + " 未找到收敛数据");
    }
    
    // 检查此步骤是否收敛
    step.converged = (step.rmsGrad < 3.0e-4 && step.maxGrad < 4.5e-4 && 
                     step.rmsStep < 1.2e-3 && step.maxStep < 1.8e-3);
}

bool BdfParser::parseFrequencies(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    if (!findFrequencySection(file)) {
        debugLog("未找到频率分析");
        return false;
    }
    
    debugLog("发现频率分析");
    
    // 跳过表头行
    std::string line;
    std::getline(file, line); // Normal frequencies line
    std::getline(file, line); // blank line
    
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        if (string_utils::contains(line, "Results of translations")) {
            break;
        }
        
        // 查找频率块表头（类似 1, 2, 3 这样的数字）
        if (!line.empty() && std::isdigit(line[0])) {
            int nFreqs = countFrequenciesInLine(line);
            if (nFreqs > 0) {
                parseFrequencyBlock(file, nFreqs, data);
            }
        }
    }
    
    infoLog("解析频率总数: " + std::to_string(data.frequencies.size()));
    
    return !data.frequencies.empty();
}

int BdfParser::countFrequenciesInLine(const std::string& line) {
    std::istringstream iss(line);
    std::string token;
    int count = 0;
    
    while (iss >> token) {
        try {
            int num = std::stoi(token);
            if (num >= 1 && num <= 100) count++;
        } catch (...) {
            break;
        }
    }
    return count;
}

void BdfParser::parseFrequencyBlock(std::ifstream& file, int nFreqs, data::ParsedData& data) {
    std::string line;
    
    // 读取 Irreps 行并提取对称性信息
    std::getline(file, line);
    std::vector<std::string> irreps;
    if (string_utils::contains(line, "Irreps")) {
        // 从行中解析 irreps
        std::istringstream iss(line);
        std::string word;
        bool foundIrreps = false;
        while (iss >> word) {
            if (foundIrreps && irreps.size() < static_cast<size_t>(nFreqs)) {
                irreps.push_back(word);
            }
            if (word == "Irreps") {
                foundIrreps = true;
            }
        }
    }
    
    // 读取频率
    std::getline(file, line);
    std::vector<double> freqValues = parseValuesFromLine(line, nFreqs);
    
    // 跳过简约质量和力常数
    std::getline(file, line); // reduced masses
    std::getline(file, line); // force constants
    
    // 读取 IR 强度
    std::getline(file, line);
    std::vector<double> irValues = parseValuesFromLine(line, nFreqs);
    
    // 创建带有对称性信息的频率模式
    int startIdx = data.frequencies.size();
    for (int i = 0; i < nFreqs; i++) {
        data::FreqMode mode;
        mode.frequency = (i < freqValues.size()) ? freqValues[i] : 0.0;
        mode.irIntensity = (i < irValues.size()) ? irValues[i] : 0.0;
        mode.irrep = (i < irreps.size()) ? irreps[i] : "A";
        data.frequencies.push_back(mode);
    }
    
    // 读取原子位移
    if (!data.optSteps.empty()) {
        int nAtoms = data.optSteps.back().atoms.size();
        debugLog("预期 " + std::to_string(nAtoms) + " 个原子的位移数据");
        
        // 为此块中的所有频率初始化位移向量
        for (int i = 0; i < nFreqs; i++) {
            if ((startIdx + i) < static_cast<int>(data.frequencies.size())) {
                data.frequencies[startIdx + i].displacements.resize(nAtoms);
                // 初始化每个原子的位移向量为3个分量
                for (int j = 0; j < nAtoms; j++) {
                    data.frequencies[startIdx + i].displacements[j].resize(3, 0.0);
                }
            }
        }
        
        // 跳过表头行（通常包含 "Atom  ZA               X         Y         Z"）
        std::getline(file, line);
        debugLog("读取潜在表头行: " + line);
        
        if (string_utils::contains(line, "Atom") && string_utils::contains(line, "ZA")) {
            debugLog("确认表头行，跳过");
            // 这是表头行，读取所有原子
            for (int iatom = 0; iatom < nAtoms; iatom++) {
                if (std::getline(file, line)) {
                    debugLog("读取原子 " + std::to_string(iatom + 1) + " 数据: " + line);
                    parseAtomDisplacements(line, startIdx, nFreqs, data);
                } else {
                    debugLog("警告: 无法读取原子 " + std::to_string(iatom + 1) + " 的位移数据");
                    break;
                }
            }
        } else {
            debugLog("不是表头行，作为第一个原子数据处理");
            // 这不是表头行，作为原子数据处理
            parseAtomDisplacements(line, startIdx, nFreqs, data);
            
            // 读取剩余的原子位移数据
            for (int iatom = 1; iatom < nAtoms; iatom++) {
                if (std::getline(file, line)) {
                    debugLog("读取原子 " + std::to_string(iatom + 1) + " 数据: " + line);
                    parseAtomDisplacements(line, startIdx, nFreqs, data);
                } else {
                    debugLog("警告: 无法读取原子 " + std::to_string(iatom + 1) + " 的位移数据");
                    break;
                }
            }
        }
    }
    
    // 跳过空行
    std::getline(file, line);
    debugLog("跳过空行: " + line);
}

std::vector<double> BdfParser::parseValuesFromLine(const std::string& line, int nVals) {
    std::vector<double> values;
    std::istringstream iss(line);
    std::string token;
    
    // 跳过初始文本
    while (iss >> token && values.size() < static_cast<size_t>(nVals)) {
        try {
            double val = std::stod(token);
            values.push_back(val);
        } catch (...) {
            // 如果转换失败就继续
        }
    }
    
    return values;
}

void BdfParser::parseAtomDisplacements(const std::string& line, int startIdx, int nFreqs, data::ParsedData& data) {
    std::istringstream iss(line);
    std::string token;
    int atomNum, za;
    
    // 读取原子编号和 ZA
    if (!(iss >> atomNum >> za)) {
        debugLog("警告: 无法从行中解析原子编号和 ZA: " + line);
        return;
    }
    
    debugLog("解析原子 " + std::to_string(atomNum) + " (ZA=" + std::to_string(za) + ") 的位移");
    
    // 读取每个频率的位移向量
    for (int ifreq = 0; ifreq < nFreqs && (startIdx + ifreq) < static_cast<int>(data.frequencies.size()); ifreq++) {
        double x, y, z;
        if (iss >> x >> y >> z) {
            // 存储位移到正确的原子位置（atomNum 是基于1的）
            int atomIdx = atomNum - 1;
            if (atomIdx >= 0 && atomIdx < static_cast<int>(data.frequencies[startIdx + ifreq].displacements.size())) {
                data.frequencies[startIdx + ifreq].displacements[atomIdx][0] = x;
                data.frequencies[startIdx + ifreq].displacements[atomIdx][1] = y;
                data.frequencies[startIdx + ifreq].displacements[atomIdx][2] = z;
                
                debugLog("  频率 " + std::to_string(startIdx + ifreq + 1) + ", 原子 " + std::to_string(atomNum) + 
                        ": (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
            } else {
                debugLog("警告: 原子索引 " + std::to_string(atomIdx) + " 对于位移存储无效");
            }
        } else {
            debugLog("警告: 无法解析原子 " + std::to_string(atomNum) + " 频率 " + std::to_string(ifreq + 1) + " 的位移值");
            break;
        }
    }
}

bool BdfParser::parseThermoData(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    // 查找热力学部分
    bool foundThermoSection = false;
    std::string line;
    while (std::getline(file, line)) {
        if (string_utils::contains(line, "Thermal Contributions to Energies")) {
            foundThermoSection = true;
            break;
        }
    }
    
    if (!foundThermoSection) {
        debugLog("未找到热力学数据");
        return false;
    }
    
    data.thermoData.hasData = true;
    debugLog("找到热力学数据");
    
    // 从当前位置继续读取
    while (std::getline(file, line)) {
        // 去除前导/尾随空白
        line = string_utils::trim(line);
        
        debugLog("处理热力学行: '" + line + "'");
        
        // 解析电子能量 - 格式: "Electronic total energy   :        -1.170752    Hartree"
        if (string_utils::contains(line, "Electronic total energy") && string_utils::contains(line, ":")) {
            size_t colonPos = line.find(":");
            std::string valueStr = line.substr(colonPos + 1);
            
            // 提取冒号后的第一个数值
            std::istringstream iss(valueStr);
            double value;
            if (iss >> value) {
                data.thermoData.electronicEnergy = value;
                debugLog("解析电子能量: " + std::to_string(data.thermoData.electronicEnergy));
            }
        }
        
        // 解析温度和压力 - 格式: "#   1    Temperature =       298.15000 Kelvin         Pressure =         1.00000 Atm"
        else if (string_utils::contains(line, "Temperature") && string_utils::contains(line, "Kelvin")) {
            // 使用简单字符串解析提取温度
            size_t tempPos = line.find("Temperature");
            if (tempPos != std::string::npos) {
                // 查找 Temperature 后的 "="
                size_t eqPos = line.find("=", tempPos);
                if (eqPos != std::string::npos) {
                    // 查找 "Kelvin" 获取结束位置
                    size_t kelvinPos = line.find("Kelvin", eqPos);
                    if (kelvinPos != std::string::npos) {
                        // 提取 "=" 和 "Kelvin" 之间的子字符串
                        std::string tempStr = line.substr(eqPos + 1, kelvinPos - eqPos - 1);
                        // 解析数值
                        data.thermoData.temperature = string_utils::toDouble(string_utils::trim(tempStr), 298.15);
                        debugLog("解析温度: " + std::to_string(data.thermoData.temperature));
                    }
                }
            }
            
            // 提取压力
            size_t pressPos = line.find("Pressure");
            if (pressPos != std::string::npos) {
                size_t eqPos = line.find("=", pressPos);
                if (eqPos != std::string::npos) {
                    size_t atmPos = line.find("Atm", eqPos);
                    if (atmPos != std::string::npos) {
                        std::string pressStr = line.substr(eqPos + 1, atmPos - eqPos - 1);
                        data.thermoData.pressure = string_utils::toDouble(string_utils::trim(pressStr), 1.0);
                        debugLog("解析压力: " + std::to_string(data.thermoData.pressure));
                    }
                }
            }
        }
        
        // 解析零点能量 - 格式: "Zero-point Energy                          :            0.010179            6.387623"
        else if (string_utils::contains(line, "Zero-point Energy") && string_utils::contains(line, ":")) {
            size_t colonPos = line.find(":");
            std::string valueStr = line.substr(colonPos + 1);
            
            std::istringstream iss(valueStr);
            double value;
            if (iss >> value) {
                data.thermoData.zpe = value;
                debugLog("解析零点能量: " + std::to_string(data.thermoData.zpe));
            }
        }
        
        // 解析热力学修正到能量 - 格式: "Thermal correction to Energy               :            0.012540            7.868837"
        else if (string_utils::contains(line, "Thermal correction to Energy") && string_utils::contains(line, ":")) {
            size_t colonPos = line.find(":");
            std::string valueStr = line.substr(colonPos + 1);
            
            std::istringstream iss(valueStr);
            double value;
            if (iss >> value) {
                data.thermoData.thermalEnergyCorr = value;
                debugLog("解析热力学修正到能量: " + std::to_string(data.thermoData.thermalEnergyCorr));
            }
        }
        
        // 解析热力学修正到焓 - 格式: "Thermal correction to Enthalpy             :            0.013484            8.461322"
        else if (string_utils::contains(line, "Thermal correction to Enthalpy") && string_utils::contains(line, ":")) {
            size_t colonPos = line.find(":");
            std::string valueStr = line.substr(colonPos + 1);
            
            std::istringstream iss(valueStr);
            double value;
            if (iss >> value) {
                data.thermoData.thermalEnthalpyCorr = value;
                debugLog("解析热力学修正到焓: " + std::to_string(data.thermoData.thermalEnthalpyCorr));
            }
        }
        
        // 解析热力学修正到Gibbs自由能 - 格式: "Thermal correction to Gibbs Free Energy    :           -0.001315           -0.825417"
        else if (string_utils::contains(line, "Thermal correction to Gibbs Free Energy") && string_utils::contains(line, ":")) {
            size_t colonPos = line.find(":");
            std::string valueStr = line.substr(colonPos + 1);
            
            std::istringstream iss(valueStr);
            double value;
            if (iss >> value) {
                data.thermoData.thermalGibbsCorr = value;
                debugLog("解析热力学修正到Gibbs自由能: " + std::to_string(data.thermoData.thermalGibbsCorr));
            }
        }
        
        // 解析收敛信息 - 格式: "  Maximum Delta-X              0.000060      0.004000            Yes"
        if (string_utils::contains(line, "Maximum Delta-X")) {
            std::istringstream iss(line);
            std::string word1, word2;
            double value, tolerance;
            std::string converged;
            
            if (iss >> word1 >> word2 >> value >> tolerance >> converged) {
                data.thermoData.maxDeltaX = value;
                data.thermoData.hasConvergenceData = true;
                debugLog("解析最大 Delta-X: " + std::to_string(data.thermoData.maxDeltaX));
            }
        }
        else if (string_utils::contains(line, "RMS Delta-X")) {
            std::istringstream iss(line);
            std::string word1, word2;
            double value, tolerance;
            std::string converged;
            
            if (iss >> word1 >> word2 >> value >> tolerance >> converged) {
                data.thermoData.rmsDeltaX = value;
                debugLog("解析 RMS Delta-X: " + std::to_string(data.thermoData.rmsDeltaX));
            }
        }
        else if (string_utils::contains(line, "Maximum Force") && !string_utils::contains(line, "Delta-X")) {
            std::istringstream iss(line);
            std::string word1, word2;
            double value, tolerance;
            std::string converged;
            
            if (iss >> word1 >> word2 >> value >> tolerance >> converged) {
                data.thermoData.maxForce = value;
                debugLog("解析最大力: " + std::to_string(data.thermoData.maxForce));
            }
        }
        else if (string_utils::contains(line, "RMS Force")) {
            std::istringstream iss(line);
            std::string word1, word2;
            double value, tolerance;
            std::string converged;
            
            if (iss >> word1 >> word2 >> value >> tolerance >> converged) {
                data.thermoData.rmsForce = value;
                debugLog("解析 RMS 力: " + std::to_string(data.thermoData.rmsForce));
            }
        }
        else if (string_utils::contains(line, "Expected Delta-E")) {
            // 解析科学记数法如 "0.27D-08"
            std::istringstream iss(line);
            std::string word1, word2;
            std::string valueStr, toleranceStr;
            std::string converged;
            
            if (iss >> word1 >> word2 >> valueStr >> toleranceStr >> converged) {
                // 将 D 记号转换为 E 记号
                std::replace(valueStr.begin(), valueStr.end(), 'D', 'E');
                try {
                    data.thermoData.expectedDeltaE = std::stod(valueStr);
                    debugLog("解析预期 Delta-E: " + std::to_string(data.thermoData.expectedDeltaE));
                } catch (...) {
                    debugLog("解析预期 Delta-E 失败: " + valueStr);
                }
            }
        }
        
        // 当到达下一个主要部分时停止 - 使用更具体的标记
        if (string_utils::contains(line, "UniMoVib job terminated")) {
            debugLog("到达热力学部分结尾: " + line);
            break;
        }
    }
    
    // 打印解析数据的摘要用于调试
    if (data.thermoData.hasData) {
        debugLog("\n=== 热力学数据摘要 ===");
        debugLog("有数据: " + std::string(data.thermoData.hasData ? "true" : "false"));
        debugLog("温度: " + std::to_string(data.thermoData.temperature) + " K");
        debugLog("压力: " + std::to_string(data.thermoData.pressure) + " atm");
        debugLog("电子能量: " + std::to_string(data.thermoData.electronicEnergy) + " Hartree");
        debugLog("零点能量: " + std::to_string(data.thermoData.zpe) + " Hartree");
        debugLog("热力学修正到能量: " + std::to_string(data.thermoData.thermalEnergyCorr) + " Hartree");
        debugLog("热力学修正到焓: " + std::to_string(data.thermoData.thermalEnthalpyCorr) + " Hartree");
        debugLog("热力学修正到Gibbs: " + std::to_string(data.thermoData.thermalGibbsCorr) + " Hartree");
        
        debugLog("\n=== 收敛数据摘要 ===");
        debugLog("有收敛数据: " + std::string(data.thermoData.hasConvergenceData ? "true" : "false"));
        if (data.thermoData.hasConvergenceData) {
            debugLog("最大 Delta-X: " + std::to_string(data.thermoData.maxDeltaX));
            debugLog("RMS Delta-X: " + std::to_string(data.thermoData.rmsDeltaX));
            debugLog("最大力: " + std::to_string(data.thermoData.maxForce));
            debugLog("RMS 力: " + std::to_string(data.thermoData.rmsForce));
            debugLog("预期 Delta-E: " + std::to_string(data.thermoData.expectedDeltaE));
        }
        debugLog("=================================");
    }
    
    return data.thermoData.hasData;
}

} // namespace parsers
} // namespace fakeg 