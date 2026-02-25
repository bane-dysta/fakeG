#include "app_runner.h"

#include <iostream>

#include "cli/argument_parser.h"
#include "string/string_utils.h"

namespace fakeg {
namespace cli {

namespace {

void printHelp(const AppSpec& spec) {
    const std::string programName = spec.programName.empty() ? "fakeg" : spec.programName;

    std::cout << "Usage: " << programName << " [options] <input_file>" << std::endl;
    std::cout << std::endl;

    if (!spec.descriptionLine.empty()) {
        std::cout << spec.descriptionLine << std::endl;
        std::cout << std::endl;
    }

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

void printVersion(const AppSpec& spec) {
    const std::string programName = spec.programName.empty() ? "fakeg" : spec.programName;
    const std::string version = spec.version.empty() ? "0.0.0" : spec.version;

    std::cout << programName << " version " << version << std::endl;
    if (!spec.author.empty()) {
        std::cout << "Author: " << spec.author << std::endl;
    }
}

} // namespace

int runAppMain(int argc,
               char* argv[],
               std::unique_ptr<parsers::ParserInterface> parser,
               const AppSpec& spec) {
    // Create application instance.
    app::FakeGApp app(std::move(parser));
    app.setProgramInfo(spec.programName, spec.version, spec.author);

    // Interactive mode.
    if (argc <= 1) {
        if (!spec.descriptionLine.empty()) {
            std::cout << spec.descriptionLine << std::endl;
        }
        if (!spec.author.empty()) {
            std::cout << "Author: " << spec.author << std::endl;
        }
        std::cout << std::endl;

        std::string inputFile;
        std::cout << (spec.inputPrompt.empty() ? "Please enter input file path: " : spec.inputPrompt);
        std::getline(std::cin, inputFile);

        inputFile = string_utils::removeQuotes(inputFile);

        if (inputFile.empty()) {
            std::cerr << "Error: No input file provided" << std::endl;
            return 1;
        }

        return app.run(inputFile) ? 0 : 1;
    }

    // CLI mode.
    ArgumentParser argParser(argc, argv);

    if (argParser.hasFlag("-h") || argParser.hasFlag("--help")) {
        printHelp(spec);
        return 0;
    }

    if (argParser.hasFlag("-v") || argParser.hasFlag("--version")) {
        printVersion(spec);
        return 0;
    }

    app.setDebugMode(argParser.hasFlag("--debug"));

    std::string outputFile = argParser.getValue("-o", "");
    if (outputFile.empty()) {
        outputFile = argParser.getValue("--output", "");
    }
    if (!outputFile.empty()) {
        app.setOutputFile(outputFile);
    }

    std::string inputFile = argParser.getPositionalArg(0);
    if (inputFile.empty()) {
        std::cerr << "Error: Please specify input file" << std::endl;
        printHelp(spec);
        return 1;
    }

    app.setInputFile(inputFile);
    return app.processFile() ? 0 : 1;
}

} // namespace cli
} // namespace fakeg
