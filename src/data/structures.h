#pragma once

#include <string>
#include <vector>
#include <map>

namespace fakeg {
namespace data {

// 原子结构
struct Atom {
    std::string symbol;
    int atomicNumber;
    double x, y, z;
    
    Atom() : atomicNumber(1), x(0.0), y(0.0), z(0.0) {}
};

// 优化步骤结构
struct OptStep {
    int stepNumber;
    std::vector<Atom> atoms;
    double energy;
    double rmsGrad, maxGrad, rmsStep, maxStep;
    bool converged;
    
    OptStep() : stepNumber(1), energy(0.0), rmsGrad(0.0), maxGrad(0.0), 
                rmsStep(0.0), maxStep(0.0), converged(false) {}
};

// 频率模式结构
struct FreqMode {
    double frequency;
    double irIntensity;
    std::string irrep;  // 对称性信息
    std::vector<std::vector<double>> displacements; // [atom][xyz]
    
    FreqMode() : frequency(0.0), irIntensity(0.0), irrep("A") {}
};

// 热力学数据结构
struct ThermoData {
    double temperature;
    double pressure;
    double electronicEnergy;
    double zpe;
    double thermalEnergyCorr;
    double thermalEnthalpyCorr;
    double thermalGibbsCorr;
    bool hasData;
    
    // 梯度收敛信息
    double maxDeltaX;
    double rmsDeltaX;
    double maxForce;
    double rmsForce;
    double expectedDeltaE;
    bool hasConvergenceData;
    
    ThermoData() : temperature(298.15), pressure(1.0), electronicEnergy(0.0),
                   zpe(0.0), thermalEnergyCorr(0.0), thermalEnthalpyCorr(0.0),
                   thermalGibbsCorr(0.0), hasData(false),
                   maxDeltaX(0.0), rmsDeltaX(0.0), maxForce(0.0), rmsForce(0.0),
                   expectedDeltaE(0.0), hasConvergenceData(false) {}
};

// 解析结果数据结构
struct ParsedData {
    std::vector<OptStep> optSteps;
    std::vector<FreqMode> frequencies;
    ThermoData thermoData;
    bool hasOpt;
    bool hasFreq;
    
    // Charge and spin information
    int charge;
    int spin;
    bool hasChargeSpinInfo;
    
    ParsedData() : hasOpt(false), hasFreq(false), charge(0), spin(1), hasChargeSpinInfo(false) {}
};

// 元素映射管理类
class ElementMap {
private:
    std::map<std::string, int> elementMap;
    void initElementMap();
    
public:
    ElementMap();
    int getAtomicNumber(const std::string& symbol) const;
    bool hasElement(const std::string& symbol) const;
};

} // namespace data
} // namespace fakeg 