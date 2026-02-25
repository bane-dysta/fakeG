#include "cli/app_runner.h"

#include "parsers/xtb_parser.h"

int main(int argc, char* argv[]) {
    fakeg::cli::AppSpec spec{
        .programName = "XtbfakeG",
        .version = "1.0.0",
        .author = "Bane Dysta & Claude 4.0",
        .descriptionLine = "XtbfakeG: Convert XTB Gaussian format output to fake Gaussian format",
        .inputPrompt = "Please enter XTB Gaussian format output file path: ",
    };

    return fakeg::cli::runAppMain(argc, argv, std::make_unique<fakeg::parsers::XtbParser>(), spec);
}
