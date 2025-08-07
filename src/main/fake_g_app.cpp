#include "fake_g_app.h"
#include "../io/file_reader.h"
#include "../parsers/parser_interface.h"
#include <iostream>
#include <filesystem>

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
    
    return true;
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

bool FakeGApp::run(int argc, char* argv[]) {
    if (!parseCommandLineArgs(argc, argv)) {
        return false;
    }
    
    return processFile();
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

void FakeGApp::printHelp() const {
    std::cout << "Usage: " << programName << " [options] <input_file>" << std::endl;
    std::cout << std::endl;
    std::cout << "Convert quantum chemistry calculation output to Gaussian format" << std::endl;
    std::cout << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --debug              Enable debug mode" << std::endl;
    std::cout << "  -o, --output FILE    Specify output filename" << std::endl;
    std::cout << "  -h, --help           Show this help message" << std::endl;
    std::cout << "  -v, --version        Show version information" << std::endl;
    std::cout << std::endl;
    std::cout << "Examples:" << std::endl;
    std::cout << "  " << programName << " input.out" << std::endl;
    std::cout << "  " << programName << " --debug -o output.log input.out" << std::endl;
}

void FakeGApp::printVersion() const {
    std::cout << programName << " version " << programVersion << std::endl;
    std::cout << "Author: " << authorInfo << std::endl;
}

bool FakeGApp::parseCommandLineArgs(int argc, char* argv[]) {
    io::ArgumentParser argParser(argc, argv);
    
    if (argParser.hasFlag("-h") || argParser.hasFlag("--help")) {
        printHelp();
        return false;
    }
    
    if (argParser.hasFlag("-v") || argParser.hasFlag("--version")) {
        printVersion();
        return false;
    }
    
    setDebugMode(argParser.hasFlag("--debug"));
    
    std::string outputFile = argParser.getValue("-o", "");
    if (outputFile.empty()) {
        outputFile = argParser.getValue("--output", "");
    }
    if (!outputFile.empty()) {
        setOutputFile(outputFile);
    }
    
    std::string inputFile = argParser.getPositionalArg(0);
    if (inputFile.empty()) {
        showErrorInfo("Please specify input file");
        printHelp();
        return false;
    }
    
    setInputFile(inputFile);
    return true;
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