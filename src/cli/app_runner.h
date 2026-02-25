#pragma once

#include <memory>
#include <string>

#include "app/fake_g_app.h"
#include "parsers/parser_interface.h"

namespace fakeg {
namespace cli {

// Metadata/configuration for a concrete FakeG application variant.
struct AppSpec {
    std::string programName;
    std::string version;
    std::string author;

    // Shown in interactive mode.
    std::string descriptionLine;
    std::string inputPrompt;
};

// Runs a FakeGApp with the provided parser and spec.
//
// Return value follows conventional main(): 0 for success, non-zero for failure.
int runAppMain(int argc,
               char* argv[],
               std::unique_ptr<parsers::ParserInterface> parser,
               const AppSpec& spec);

} // namespace cli
} // namespace fakeg
