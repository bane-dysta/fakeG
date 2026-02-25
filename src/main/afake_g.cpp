#include "cli/app_runner.h"

#include "parsers/amesp_parser.h"

int main(int argc, char* argv[]) {
    fakeg::cli::AppSpec spec{
        .programName = "AfakeG",
        .version = "1.0.0",
        .author = "Bane Dysta & Claude 4.0",
        .descriptionLine = "AfakeG: Convert AMESP output to fake Gaussian format",
        .inputPrompt = "Please enter AMESP output file path: ",
    };

    return fakeg::cli::runAppMain(argc, argv, std::make_unique<fakeg::parsers::AmespParser>(), spec);
}
