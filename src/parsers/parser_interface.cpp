#include "parser_interface.h"
#include <iostream>

namespace fakeg {
namespace parsers {

ParserInterface::ParserInterface() : logger(nullptr) {
    elementMap = std::make_shared<data::ElementMap>();
}

void ParserInterface::setLogger(logger::Logger* logger) {
    this->logger = logger;
}

void ParserInterface::debugLog(const std::string& message) const {
    if (logger) {
        logger->debug(message);
    }
}

void ParserInterface::infoLog(const std::string& message) const {
    if (logger) {
        logger->info(message);
    } else {
        std::cout << message << std::endl;
    }
}

void ParserInterface::errorLog(const std::string& message) const {
    if (logger) {
        logger->error(message);
    } else {
        std::cerr << "ERROR: " << message << std::endl;
    }
}

} // namespace parsers
} // namespace fakeg 