#include "cli/app_runner.h"

#include "parsers/xyz_parser.h"

int main(int argc, char* argv[]) {
    fakeg::cli::AppSpec spec{
        .programName = "XfakeG",
        .version = "1.0.0",
        .author = "Bane Dysta & Claude 4.0",
        .descriptionLine = "XfakeG: Convert XYZ/TRJ trajectory to fake Gaussian format",
        .inputPrompt = "Please enter XYZ/TRJ trajectory file path: ",
    };

    return fakeg::cli::runAppMain(argc, argv, std::make_unique<fakeg::parsers::XyzParser>(), spec);
}
