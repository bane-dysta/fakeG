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

// 轨道跃迁结构
struct OrbitalTransition {
    int fromOrb;
    int toOrb;
    double coefficient;
    bool isAlpha; // true for alpha, false for beta (for unrestricted calculations)
    bool isForward; // true for ->, false for <-
    
    OrbitalTransition() : fromOrb(0), toOrb(0), coefficient(0.0), isAlpha(true), isForward(true) {}
};

// 激发态结构
struct ExcitedState {
    int stateNumber;
    std::string symmetry;        // e.g., "Singlet-A'", "Triplet-A""
    double excitationEnergy_eV;  // 激发能量 (eV)
    double wavelength_nm;        // 波长 (nm)
    double oscillatorStrength;   // 振荡强度 f
    double s2Value;             // <S**2> 值
    std::vector<OrbitalTransition> transitions; // 轨道跃迁列表
    
    // 可选信息
    bool hasOptimizationInfo;    // 是否有优化相关信息
    bool hasTotalEnergy;         // 是否有总能量
    double totalEnergy;          // Total Energy, E(TD-HF/TD-DFT)
    std::string additionalInfo;  // 额外信息，如"This state for optimization..."
    
    ExcitedState() : stateNumber(1), symmetry("Singlet-A"), excitationEnergy_eV(0.0),
                     wavelength_nm(0.0), oscillatorStrength(0.0), s2Value(0.0),
                     hasOptimizationInfo(false), hasTotalEnergy(false), totalEnergy(0.0) {}
};

// TDDFT数据结构
struct TDDFTData {
    std::vector<ExcitedState> excitedStates;
    bool hasData;
    
    TDDFTData() : hasData(false) {}
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
    
    // TDDFT information
    std::vector<TDDFTData> tddftData; // 每个优化步骤对应一个TDDFT数据
    bool hasTDDFT;
    
    ParsedData() : hasOpt(false), hasFreq(false), charge(0), spin(1), hasChargeSpinInfo(false), hasTDDFT(false) {}
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