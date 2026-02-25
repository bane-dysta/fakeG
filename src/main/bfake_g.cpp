#include "cli/app_runner.h"

#include "parsers/bdf_parser.h"

int main(int argc, char* argv[]) {
    fakeg::cli::AppSpec spec{
        .programName = "BfakeG",
        .version = "1.0.0",
        .author = "Bane Dysta & Claude 4.0",
        .descriptionLine = "BfakeG: Convert BDF output to fake Gaussian format",
        .inputPrompt = "Please enter BDF output file path: ",
    };

    return fakeg::cli::runAppMain(argc, argv, std::make_unique<fakeg::parsers::BdfParser>(), spec);
}
