#include "fake_g_app.h"

#include <filesystem>
#include <iostream>

namespace fakeg {
namespace app {

FakeGApp::FakeGApp() : debugMode(false), appLogger(false, logger::LogLevel::INFO) {
    programName = "FakeG";
    programVersion = "1.0.0";
    authorInfo = "FakeG Project";
    
    // 设置全局logger
    logger::globalLogger = appLogger;
}

FakeGApp::FakeGApp(std::unique_ptr<parsers::ParserInterface> parser) : FakeGApp() {
    setParser(std::move(parser));
}

void FakeGApp::setParser(std::unique_ptr<parsers::ParserInterface> parser) {
    this->parser = std::move(parser);
    if (this->parser) {
        this->parser->setLogger(&appLogger);
    }
}

void FakeGApp::setProgramInfo(const std::string& name, const std::string& version, const std::string& author) {
    programName = name;
    programVersion = version;
    authorInfo = author;
    
    // 更新 writer 的程序信息
    writer.setProgramInfo(name, version, author);
}

void FakeGApp::setDebugMode(bool enable) {
    debugMode = enable;
    appLogger.setDebugMode(enable);
    
    // 更新全局logger
    logger::globalLogger = appLogger;
}

void FakeGApp::setInputFile(const std::string& filename) {
    inputFilename = filename;
}

void FakeGApp::setOutputFile(const std::string& filename) {
    outputFilename = filename;
}

bool FakeGApp::initialize() {
    if (!parser) {
        showErrorInfo("No parser set");
        return false;
    }
    
    if (inputFilename.empty()) {
        showErrorInfo("No input file specified");
        return false;
    }
    
    if (outputFilename.empty()) {
        outputFilename = io::GaussianWriter::generateOutputFilename(inputFilename, "_fake");
    }

    // Ensure output directory exists (important when output is in a non-existent folder).
    return setupOutput();
}

bool FakeGApp::processFile() {
    if (!initialize()) {
        return false;
    }
    
    appLogger.info("Starting to process file: " + inputFilename);
    appLogger.debug("Using parser: " + parser->getParserName() + " v" + parser->getParserVersion());
    
    // 打开输入文件
    io::FileReader reader(inputFilename);
    if (!reader.isOpen()) {
        showErrorInfo("Cannot open input file: " + inputFilename);
        return false;
    }
    
    // 验证输入文件
    if (!parser->validateInput(inputFilename)) {
        showErrorInfo("Input file format is incorrect");
        return false;
    }
    
    // 解析文件
    data::ParsedData parsedData;
    if (!parser->parse(reader, parsedData)) {
        showErrorInfo("Failed to parse file");
        return false;
    }
    
    // 显示进度信息
    showProgressInfo(parsedData);
    
    // 生成输出
    writer.setOutputFilename(outputFilename);
    if (!writer.writeGaussianOutput(parsedData)) {
        showErrorInfo("Failed to write output file: " + outputFilename);
        return false;
    }
    
    appLogger.info("Successfully generated output file: " + outputFilename);
    return true;
}

bool FakeGApp::validateOutput() {
    return writer.validateOutput(outputFilename);
}

bool FakeGApp::run(const std::string& inputFilename, bool debugMode) {
    setInputFile(inputFilename);
    setDebugMode(debugMode);
    
    return processFile();
}

std::string FakeGApp::getInputFile() const {
    return inputFilename;
}

std::string FakeGApp::getOutputFile() const {
    return outputFilename;
}

bool FakeGApp::isDebugMode() const {
    return debugMode;
}

bool FakeGApp::setupOutput() {
    if (outputFilename.empty()) {
        outputFilename = io::GaussianWriter::generateOutputFilename(inputFilename, "_fake");
    }
    
    // 检查输出目录是否存在
    std::filesystem::path outputPath(outputFilename);
    std::filesystem::path outputDir = outputPath.parent_path();
    
    if (!outputDir.empty() && !std::filesystem::exists(outputDir)) {
        try {
            std::filesystem::create_directories(outputDir);
        } catch (const std::exception& e) {
            showErrorInfo("Cannot create output directory: " + outputDir.string());
            return false;
        }
    }
    
    return true;
}

void FakeGApp::showProgressInfo(const data::ParsedData& data) {
    if (data.hasOpt && !data.optSteps.empty()) {
        appLogger.info("Found optimization calculation with " + std::to_string(data.optSteps.size()) + " steps");
    } else if (!data.optSteps.empty()) {
        appLogger.info("Found single point calculation");
    }
    
    if (data.hasFreq && !data.frequencies.empty()) {
        appLogger.info("Found frequency calculation with " + std::to_string(data.frequencies.size()) + " frequencies");
    }
    
    if (data.thermoData.hasData) {
        appLogger.info("Found thermodynamic data");
    }
}

void FakeGApp::showErrorInfo(const std::string& error) {
    appLogger.error(error);
}

} // namespace app
} // namespace fakeg 