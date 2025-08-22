#include "amesp_parser.h"
#include "../string/string_utils.h"
#include <sstream>

namespace fakeg {
namespace parsers {

AmespParser::AmespParser() = default;

bool AmespParser::parse(io::FileReader& reader, data::ParsedData& data) {
    auto& file = reader.getStream();
    
    debugLog("Starting AMESP file parsing: " + reader.getFilename());
    
    // 检查是否有TD-DFT数据
    string_utils::LineProcessor::resetToBeginning(file);
    if (string_utils::LineProcessor::findLineFromBeginning(file, "E[Eexc]")) {
        data.hasTDDFT = true;
        infoLog("Found TD-DFT data (E[Eexc])");
    }
    
    // 检查优化
    string_utils::LineProcessor::resetToBeginning(file);
    if (string_utils::LineProcessor::findLineFromBeginning(file, "Geom Opt Step:")) {
        data.hasOpt = true;
        infoLog("Found geometry optimization");
        if (!parseOptimizationSteps(file, data)) {
            errorLog("Optimization steps parsing failed");
            return false;
        }
    } else {
        // 单点计算
        infoLog("Single point calculation detected");
        if (!parseSinglePoint(file, data)) {
            errorLog("Single point calculation parsing failed");
            return false;
        }
    }
    
    // 解析TD-DFT数据
    if (data.hasTDDFT) {
        if (!parseTDDFT(file, data)) {
            errorLog("TD-DFT data parsing failed");
            return false;
        }
        infoLog("TD-DFT data parsing completed");
    }
    
    // 解析频率
    if (parseFrequencies(file, data)) {
        data.hasFreq = true;
        infoLog("Frequency parsing completed");
    }
    
    // 解析热力学数据
    if (parseThermoData(file, data)) {
        infoLog("Thermodynamic data parsing completed");
    }
    
    debugLog("AMESP file parsing completed");
    return true;
}

bool AmespParser::validateInput(const std::string& filename) {
    // 只检查文件是否存在，不检查扩展名
    std::ifstream file(filename);
    if (!file.is_open()) {
        errorLog("Cannot open file: " + filename);
        return false;
    }
    
    // 用户指定的文件就是想要转换的文件，不需要额外验证
    return true;
}

std::string AmespParser::getParserName() const {
    return "AmespParser";
}

std::string AmespParser::getParserVersion() const {
    return "1.0.0";
}

std::vector<std::string> AmespParser::getSupportedKeywords() const {
    return {"OPT", "FREQ", "SP", "SINGLE_POINT", "OPTIMIZATION", "FREQUENCY"};
}

bool AmespParser::parseOptimizationSteps(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    std::string line;
    while (std::getline(file, line)) {
        if (line.find("Geom Opt Step:") != std::string::npos) {
            data::OptStep step;
            step.energy = 0.0;
            step.rmsGrad = step.maxGrad = step.rmsStep = step.maxStep = 0.0;
            step.converged = false;
            
            // 提取步骤编号
            std::istringstream iss(line);
            std::string dummy1, dummy2, dummy3;
            if (iss >> dummy1 >> dummy2 >> dummy3 >> step.stepNumber) {
                debugLog("Processing optimization step " + std::to_string(step.stepNumber));
            }
            
            // 查找并解析几何
            if (string_utils::LineProcessor::findLine(file, "Current Geometry(angstroms):")) {
                std::getline(file, line); // 跳过头行
                parseGeometry(file, step.atoms);
            }
            
            // 解析能量
            step.energy = parseEnergyFromCurrentPosition(file);
            
            // 解析收敛性
            parseConvergence(file, step);
            
            if (!step.atoms.empty()) {
                data.optSteps.push_back(step);
                debugLog("Added step " + std::to_string(step.stepNumber) + 
                        " containing " + std::to_string(step.atoms.size()) + " atoms");
            }
        }
    }
    
    infoLog("Total optimization steps: " + std::to_string(data.optSteps.size()));
    return !data.optSteps.empty();
}

bool AmespParser::parseSinglePoint(std::ifstream& file, data::ParsedData& data) {
    data::OptStep step;
    step.stepNumber = 1;
    step.converged = true;
    
    // 查找几何
    string_utils::LineProcessor::resetToBeginning(file);
    if (string_utils::LineProcessor::findLineFromBeginning(file, "Current Geometry(angstroms):")) {
        std::string line;
        std::getline(file, line); // 跳过头行
        parseGeometry(file, step.atoms);
    }
    
    // 解析能量
    step.energy = parseEnergyFromCurrentPosition(file);
    
    if (!step.atoms.empty()) {
        data.optSteps.push_back(step);
        return true;
    }
    
    return false;
}

bool AmespParser::parseFrequencies(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    if (!string_utils::LineProcessor::findLineFromBeginning(file, "========================== Frequency ===========================")) {
        debugLog("Frequency analysis not found");
        return false;
    }
    
    debugLog("Found frequency analysis");
    
    // 查找谐振频率
    if (!string_utils::LineProcessor::findLine(file, "Harmonic frequencies(cm-1):")) {
        return false;
    }
    
    // 跳过空行
    std::string line;
    std::getline(file, line);
    
    // 读取频率值
    std::vector<double> freqValues;
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        if (line.empty() || line.find("Zero-point") != std::string::npos) {
            break;
        }
        
        std::istringstream iss(line);
        int index;
        double freq;
        if (iss >> index >> freq) {
            freqValues.push_back(freq);
        }
    }
    
    debugLog("Parsed " + std::to_string(freqValues.size()) + " frequencies");
    
    // 查找IR强度
    std::vector<double> irValues;
    if (string_utils::LineProcessor::findLine(file, ">>>>>>>>>>>>>>>> IR spectrum (T^2,KM/Mole) <<<<<<<<<<<<<<<<")) {
        std::getline(file, line); // 跳过空行
        std::getline(file, line); // 跳过表头 "freq(cm^-1)     T^2         Tx         Ty         Tz"
        
        for (size_t i = 0; i < freqValues.size(); i++) {
            if (std::getline(file, line)) {
                std::istringstream iss(line);
                int index;
                double freq, intensity;
                if (iss >> index >> freq >> intensity) {
                    irValues.push_back(intensity);
                    debugLog("Frequency " + std::to_string(i+1) + ": " + std::to_string(freq) + " cm-1, IR intensity: " + std::to_string(intensity));
                } else {
                    irValues.push_back(0.0);
                }
            } else {
                irValues.push_back(0.0);
            }
        }
    } else {
        debugLog("IR spectrum data not found");
        // 如果没有找到IR数据，用0填充
        irValues.resize(freqValues.size(), 0.0);
    }
    
    // 创建频率模式
    for (size_t i = 0; i < freqValues.size(); i++) {
        data::FreqMode mode;
        mode.frequency = freqValues[i];
        mode.irIntensity = (i < irValues.size()) ? irValues[i] : 0.0;
        mode.irrep = "A"; // 默认对称性
        data.frequencies.push_back(mode);
    }
    
    debugLog("Frequency parsing completed, " + std::to_string(data.frequencies.size()) + " modes");
    
    // 解析法向模式
    parseNormalModes(file, data);
    
    return !data.frequencies.empty();
}

bool AmespParser::parseThermoData(std::ifstream& file, data::ParsedData& data) {
    std::string line;
    
    // 重置文件位置到开始
    string_utils::LineProcessor::resetToBeginning(file);
    
    // 首先尝试找到热力学摘要部分
    if (string_utils::LineProcessor::findLine(file, ">>>>>>>>>>> Summary of Thermodynamic Quantities <<<<<<<<<<<<<")) {
        data.thermoData.hasData = true;
        debugLog("Found thermodynamic summary section");
    } else {
        // 如果没有找到摘要部分，从头开始查找单独的热力学值
        string_utils::LineProcessor::resetToBeginning(file);
    }
    
    // 解析文件中的所有热力学数据
    string_utils::LineProcessor::resetToBeginning(file);
    
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        // 解析温度
        if (string_utils::contains(line, "Temperature:")) {
            std::istringstream iss(line);
            std::string dummy;
            double temp;
            if (iss >> dummy >> temp) {
                data.thermoData.temperature = temp;
                data.thermoData.hasData = true;
                debugLog("Found temperature: " + std::to_string(temp) + " K");
            }
        }
        // 解析压力
        else if (string_utils::contains(line, "Pressure:")) {
            std::istringstream iss(line);
            std::string dummy;
            double press;
            if (iss >> dummy >> press) {
                data.thermoData.pressure = press;
                data.thermoData.hasData = true;
                debugLog("Found pressure: " + std::to_string(press) + " atm");
            }
        }
        // 解析零点振动能
        else if (string_utils::contains(line, "Zero-point vibrational energy:")) {
            std::istringstream iss(line);
            std::string dummy1, dummy2, dummy3;
            double zpe;
            if (iss >> dummy1 >> dummy2 >> dummy3 >> zpe) {
                data.thermoData.zpe = zpe;
                data.thermoData.hasData = true;
                debugLog("Found zero-point energy: " + std::to_string(zpe) + " Hartree");
            }
        }
        // 解析热力学修正到U(T) - 对应"Thermal correction to Energy"
        else if (string_utils::contains(line, "Thermal correction to U(T):")) {
            std::istringstream iss(line);
            std::string dummy1, dummy2, dummy3, dummy4;
            double value;
            if (iss >> dummy1 >> dummy2 >> dummy3 >> dummy4 >> value) {
                data.thermoData.thermalEnergyCorr = value;
                data.thermoData.hasData = true;
                debugLog("Found thermal correction to U(T): " + std::to_string(value) + " Hartree");
            }
        }
        // 解析热力学修正到H(T) - 对应"Thermal correction to Enthalpy"
        else if (string_utils::contains(line, "Thermal correction to H(T):")) {
            std::istringstream iss(line);
            std::string dummy1, dummy2, dummy3, dummy4;
            double value;
            if (iss >> dummy1 >> dummy2 >> dummy3 >> dummy4 >> value) {
                data.thermoData.thermalEnthalpyCorr = value;
                data.thermoData.hasData = true;
                debugLog("Found thermal correction to H(T): " + std::to_string(value) + " Hartree");
            }
        }
        // 解析热力学修正到G(T) - 对应"Thermal correction to Gibbs Free Energy"
        else if (string_utils::contains(line, "Thermal correction to G(T):")) {
            std::istringstream iss(line);
            std::string dummy1, dummy2, dummy3, dummy4;
            double value;
            if (iss >> dummy1 >> dummy2 >> dummy3 >> dummy4 >> value) {
                data.thermoData.thermalGibbsCorr = value;
                data.thermoData.hasData = true;
                debugLog("Found thermal correction to G(T): " + std::to_string(value) + " Hartree");
            }
        }
        // 解析最终电子能量
        else if (string_utils::contains(line, "Final Energy:")) {
            std::istringstream iss(line);
            std::string dummy1, dummy2;
            double energy;
            if (iss >> dummy1 >> dummy2 >> energy) {
                data.thermoData.electronicEnergy = energy;
                data.thermoData.hasData = true;
                debugLog("Found final energy: " + std::to_string(energy) + " Hartree");
            }
        }
    }
    
    if (data.thermoData.hasData) {
        debugLog("Thermodynamic data parsing completed:");
        debugLog("  Temperature: " + std::to_string(data.thermoData.temperature) + " K");
        debugLog("  Pressure: " + std::to_string(data.thermoData.pressure) + " atm");
        debugLog("  Electronic energy: " + std::to_string(data.thermoData.electronicEnergy) + " Hartree");
        debugLog("  Zero-point energy: " + std::to_string(data.thermoData.zpe) + " Hartree");
        debugLog("  Thermal correction to energy: " + std::to_string(data.thermoData.thermalEnergyCorr) + " Hartree");
        debugLog("  Thermal correction to enthalpy: " + std::to_string(data.thermoData.thermalEnthalpyCorr) + " Hartree");
        debugLog("  Thermal correction to Gibbs: " + std::to_string(data.thermoData.thermalGibbsCorr) + " Hartree");
    }
    
    return data.thermoData.hasData;
}

void AmespParser::parseGeometry(std::ifstream& file, std::vector<data::Atom>& atoms) {
    atoms.clear();
    
    std::string line;
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        // 停在分隔线
        if (line.find("----------------------------------------------------------------") != std::string::npos) {
            break;
        }
        
        // 解析原子行：Element X Y Z
        if (!line.empty()) {
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
                atoms.push_back(atom);
                
                debugLog("Read atom: " + element + " (" + std::to_string(atom.atomicNumber) +
                        ") at (" + std::to_string(x) + ", " + std::to_string(y) + ", " + std::to_string(z) + ")");
            }
        }
    }
}

double AmespParser::parseEnergyFromCurrentPosition(std::ifstream& file) {
    std::string line;
    double energy = 0.0;
    
    // 直接查找E[DFT]，不再根据E[Eexc]的存在与否来选择
    std::string targetPattern = "E[DFT]";
    
    while (std::getline(file, line)) {
        if (line.find(targetPattern) != std::string::npos) {
            size_t pos = line.find("=");
            if (pos != std::string::npos) {
                std::string energyStr = string_utils::trim(line.substr(pos + 1));
                energy = string_utils::toDouble(energyStr);
                debugLog("Found energy " + targetPattern + ": " + std::to_string(energy));
                return energy;
            }
        }
        
        if (line.find("Geom Opt Step:") != std::string::npos) {
            break;
        }
    }
    
    return energy;
}

void AmespParser::parseConvergence(std::ifstream& file, data::OptStep& step) {
    std::string line;
    
    // 查找收敛部分
    while (std::getline(file, line)) {
        if (line.find("Geometry Convergence:") != std::string::npos) {
            debugLog("Found convergence section, step " + std::to_string(step.stepNumber));
            break;
        }
        if (line.find("Geom Opt Step:") != std::string::npos || 
            line.find("Geometry Optimization Converged") != std::string::npos) {
            return;
        }
    }
    
    // 查找收敛表
    while (std::getline(file, line)) {
        if (line.find("Item              Value        Threshold       Converged?") != std::string::npos) {
            std::getline(file, line); // 跳过分隔线
            break;
        }
        if (line.find("Geom Opt Step:") != std::string::npos) {
            return;
        }
    }
    
    // 解析收敛值
    for (int i = 0; i < 4; i++) {
        if (std::getline(file, line)) {
            line = string_utils::trim(line);
            std::istringstream iss(line);
            std::string word1, word2;
            double value, threshold;
            std::string converged;
            
            if (iss >> word1 >> word2 >> value >> threshold >> converged) {
                if (word1 == "RMS" && word2 == "Force") {
                    step.rmsGrad = value;
                } else if (word1 == "Max" && word2 == "Force") {
                    step.maxGrad = value;
                } else if (word1 == "RMS" && word2 == "Step") {
                    step.rmsStep = value;
                } else if (word1 == "Max" && word2 == "Step") {
                    step.maxStep = value;
                }
            }
        }
    }
    
    // 检查是否收敛
    step.converged = (step.rmsGrad < 0.0003 && step.maxGrad < 0.00045 && 
                     step.rmsStep < 0.0012 && step.maxStep < 0.0018);
    
    debugLog("Step " + std::to_string(step.stepNumber) + " convergence info: " +
            "RMS gradient=" + std::to_string(step.rmsGrad) + ", " +
            "Max gradient=" + std::to_string(step.maxGrad) + ", " +
            "Converged=" + (step.converged ? "Yes" : "No"));
}

void AmespParser::parseNormalModes(std::ifstream& file, data::ParsedData& data) {
    std::string line;
    
    // 重置文件位置到开始
    string_utils::LineProcessor::resetToBeginning(file);
    
    if (!string_utils::LineProcessor::findLine(file, "Normal Modes:")) {
        debugLog("Normal Modes section not found");
        return;
    }
    
    if (data.optSteps.empty()) {
        debugLog("No geometry information, cannot parse normal modes");
        return;
    }
    
    int nAtoms = data.optSteps.back().atoms.size();
    int nFreqs = data.frequencies.size();
    
    debugLog("Starting normal mode parsing, number of atoms: " + std::to_string(nAtoms) + ", number of frequencies: " + std::to_string(nFreqs));
    
    // 初始化位移向量
    for (int i = 0; i < nFreqs; i++) {
        data.frequencies[i].displacements.resize(nAtoms);
        for (int j = 0; j < nAtoms; j++) {
            data.frequencies[i].displacements[j].resize(3, 0.0);
        }
    }
    
    // 跳过空行和表头
    std::getline(file, line);
    std::getline(file, line); // header line with mode numbers
    
    // 解析位移数据
    int currentModeStart = 0;
    int modesInBlock = std::min(5, nFreqs); // 每块最多5个模式
    
    while (currentModeStart < nFreqs) {
        // 确定这个块中有多少个模式
        modesInBlock = std::min(5, nFreqs - currentModeStart);
        
        // 读取原子位移数据
        for (int atomIdx = 0; atomIdx < nAtoms * 3; atomIdx++) {
            if (!std::getline(file, line)) break;
            
            std::istringstream iss(line);
            int index, atom;
            std::string coord;
            if (!(iss >> index >> atom >> coord)) continue;
            
            int actualAtom = atom - 1; // 转换为0基索引
            int coordIdx = (coord == "X") ? 0 : (coord == "Y") ? 1 : 2;
            
            if (actualAtom >= 0 && actualAtom < nAtoms && coordIdx >= 0 && coordIdx < 3) {
                for (int modeIdx = 0; modeIdx < modesInBlock; modeIdx++) {
                    double displacement;
                    if (iss >> displacement) {
                        int globalModeIdx = currentModeStart + modeIdx;
                        if (globalModeIdx < nFreqs) {
                            data.frequencies[globalModeIdx].displacements[actualAtom][coordIdx] = displacement;
                        }
                    }
                }
            }
        }
        
        currentModeStart += modesInBlock;
        
        // 如果还有下一个块，跳过空行和表头
        if (currentModeStart < nFreqs) {
            std::getline(file, line); // 空行
            std::getline(file, line); // 表头行
        }
    }
    
    debugLog("Normal mode parsing completed, processed " + std::to_string(nFreqs) + " frequencies");
}

bool AmespParser::findOptimizationSection(std::ifstream& file) {
    return string_utils::LineProcessor::findLineFromBeginning(file, "Geom Opt Step:");
}

bool AmespParser::findFrequencySection(std::ifstream& file) {
    return string_utils::LineProcessor::findLineFromBeginning(file, "========================== Frequency ===========================");
}

bool AmespParser::findThermoSection(std::ifstream& file) {
    string_utils::LineProcessor::resetToBeginning(file);
    return string_utils::LineProcessor::findLineFromBeginning(file, "Temperature:") ||
           string_utils::LineProcessor::findLineFromBeginning(file, "Zero-point vibrational energy:");
}

bool AmespParser::parseTDDFT(std::ifstream& file, data::ParsedData& data) {
    string_utils::LineProcessor::resetToBeginning(file);
    
    // 为每个优化步骤或单点计算查找对应的TD-DFT数据
    int expectedSteps = data.optSteps.size();
    data.tddftData.resize(expectedSteps);
    
    debugLog("Parsing TD-DFT data for " + std::to_string(expectedSteps) + " steps");
    
    int currentStep = 0;
    std::string line;
    
    while (std::getline(file, line) && currentStep < expectedSteps) {
        // 查找TD-DFT块的开始标记
        if (line.find("========= Excitation energies and oscillator strengths =========") != std::string::npos) {
            debugLog("Found TD-DFT section for step " + std::to_string(currentStep + 1));
            
            // 先查找这个步骤对应的E[Eexc]值
            double eExcValue = 0.0;
            std::streampos currentPos = file.tellg();
            
            // 向前查找E[Eexc]值
            while (std::getline(file, line)) {
                if (line.find("E[Eexc]") != std::string::npos) {
                    size_t pos = line.find("=");
                    if (pos != std::string::npos) {
                        std::string energyStr = string_utils::trim(line.substr(pos + 1));
                        eExcValue = string_utils::toDouble(energyStr);
                        debugLog("Found E[Eexc] for step " + std::to_string(currentStep + 1) + ": " + std::to_string(eExcValue));
                        break;
                    }
                }
                // 如果遇到下一个TD-DFT块或其他标记，停止搜索
                if (line.find("========= Excitation energies and oscillator strengths =========") != std::string::npos ||
                    line.find("Geom Opt Step:") != std::string::npos) {
                    break;
                }
            }
            
            // 重置到TD-DFT块开始位置
            file.clear();
            file.seekg(currentPos);
            
            // 解析这个TD-DFT块，传入E[Eexc]值
            data.tddftData[currentStep] = parseTDDFTSection(file, eExcValue);
            data.tddftData[currentStep].hasData = !data.tddftData[currentStep].excitedStates.empty();
            
            if (data.tddftData[currentStep].hasData) {
                debugLog("Parsed " + std::to_string(data.tddftData[currentStep].excitedStates.size()) + 
                        " excited states for step " + std::to_string(currentStep + 1));
            }
            
            currentStep++;
        }
    }
    
    debugLog("TD-DFT parsing completed, processed " + std::to_string(currentStep) + " steps");
    return currentStep > 0;
}

data::TDDFTData AmespParser::parseTDDFTSection(std::ifstream& file, double eExcValue) {
    data::TDDFTData tddftData;
    std::string line;
    
    while (std::getline(file, line)) {
        line = string_utils::trim(line);
        
        // 检查是否是激发态开始行
        if (line.find("State") != std::string::npos && line.find(":") != std::string::npos && 
            line.find("E =") != std::string::npos) {
            
            data::ExcitedState excitedState = parseExcitedState(file, line, eExcValue);
            if (excitedState.stateNumber > 0) {
                tddftData.excitedStates.push_back(excitedState);
                debugLog("Parsed excited state " + std::to_string(excitedState.stateNumber));
            }
        }
        
        // 检查是否到达TD-DFT块结束
        if (line.find("Time of TDDFT") != std::string::npos || 
            line.find("================================================================") != std::string::npos) {
            break;
        }
    }
    
    return tddftData;
}

data::ExcitedState AmespParser::parseExcitedState(std::ifstream& file, const std::string& stateLine, double eExcValue) {
    data::ExcitedState excitedState;
    
    // 解析状态行：State    1 : E =    7.1627 eV     173.097 nm      57770.95 cm-1
    std::istringstream iss(stateLine);
    std::string dummy;
    if (iss >> dummy >> excitedState.stateNumber >> dummy >> dummy >> dummy >> excitedState.excitationEnergy_eV 
        >> dummy >> excitedState.wavelength_nm >> dummy) {
        
        debugLog("Parsing excited state " + std::to_string(excitedState.stateNumber) + 
                ", E = " + std::to_string(excitedState.excitationEnergy_eV) + " eV");
        
        // 设置默认对称性（AMESP输出中没有明确的对称性信息）
        excitedState.symmetry = "Singlet-A";
        
        // 解析轨道跃迁和其他信息
        std::string line;
        while (std::getline(file, line)) {
            line = string_utils::trim(line);
            
            // 空行可能表示激发态结束
            if (line.empty()) {
                break;
            }
            
            // 解析轨道跃迁：11 -->   13      0.5002429 或 10 <--   12     -0.5002429
            if (line.find("-->") != std::string::npos || line.find("<--") != std::string::npos) {
                std::istringstream transIss(line);
                int fromOrb, toOrb;
                std::string arrow;
                double coeff;
                
                if (transIss >> fromOrb >> arrow >> toOrb >> coeff) {
                    data::OrbitalTransition transition;
                    transition.fromOrb = fromOrb;
                    transition.toOrb = toOrb;
                    transition.coefficient = coeff; // 保持原始系数，包括负号
                    transition.isForward = (arrow == "-->"); // --> 为true，<-- 为false
                    excitedState.transitions.push_back(transition);
                    
                    debugLog("  Transition: " + std::to_string(fromOrb) + " " + arrow + " " + 
                            std::to_string(toOrb) + " (" + std::to_string(coeff) + ")");
                }
            }
            
            // 解析E(TD)行：E(TD) =   -188.290813700      <S**2>= 0.000     f=  0.0000
            else if (line.find("E(TD)") != std::string::npos) {
                std::istringstream etdIss(line);
                std::string etd, eq;
                double totalEnergy;
                
                if (etdIss >> etd >> eq >> totalEnergy) {
                    // 检查是否为追踪态 (E(TD) = E[Eexc])
                    const double tolerance = 1e-9; // 浮点数比较容差
                    bool isTrackedState = (std::abs(totalEnergy - eExcValue) < tolerance);
                    
                    if (isTrackedState) {
                        excitedState.hasOptimizationInfo = true;
                        excitedState.hasTotalEnergy = true;
                        excitedState.totalEnergy = totalEnergy;
                        excitedState.additionalInfo = "Copying the excited state density for this state as the 1-particle RhoCI density.";
                        
                        debugLog("  This is the tracked state (E(TD) = E[Eexc])");
                    }
                    
                    // 查找<S**2>值
                    size_t s2Pos = line.find("<S**2>=");
                    if (s2Pos != std::string::npos) {
                        std::string s2Str = line.substr(s2Pos + 7);
                        size_t spacePos = s2Str.find(' ');
                        if (spacePos != std::string::npos) {
                            excitedState.s2Value = string_utils::toDouble(s2Str.substr(0, spacePos));
                        }
                    }
                    
                    // 查找振荡强度f值
                    size_t fPos = line.find("f=");
                    if (fPos != std::string::npos) {
                        std::string fStr = line.substr(fPos + 2);
                        excitedState.oscillatorStrength = string_utils::toDouble(string_utils::trim(fStr));
                    }
                    
                    debugLog("  Total energy: " + std::to_string(totalEnergy) + 
                            ", <S**2>: " + std::to_string(excitedState.s2Value) +
                            ", f: " + std::to_string(excitedState.oscillatorStrength) +
                            ", tracked: " + (isTrackedState ? "Yes" : "No"));
                }
                break; // E(TD)行通常是激发态的最后一行
            }
            
            // 如果遇到下一个State行，停止解析当前激发态
            else if (line.find("State") != std::string::npos && line.find(":") != std::string::npos) {
                // 将文件指针回退到这一行的开始
                std::streampos currentPos = file.tellg();
                file.seekg(currentPos - static_cast<std::streampos>(line.length() + 1));
                break;
            }
        }
    }
    
    return excitedState;
}

} // namespace parsers
} // namespace fakeg 