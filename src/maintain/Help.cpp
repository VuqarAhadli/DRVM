#include "maintain/Help.hpp"
#include "maintain/ProgramInfo.hpp"

void printHelp()
{
    std::cout << "DRVM is a small JVM and java class file disassembler mainly designed for reverse engineering.\n"
              << "Version: " << DRVM_VERSION << "\n"
              << "Source code: https://github.com/VuqarAhadli/DRVM\n"
              << "Options:\n"
              << "  --dump            Dump the class file information.\n"
              << "  --run             Run the class file.\n"
              << "  --trace           Enable tracing of execution.\n"
              << "  --debug           Enable debug mode for detailed output.\n"
              << "  --help            Show this help message.\n"
              << "  --version         Show version information.\n"
              << "  --license         Show license information.\n"
              << "  --contributors    Show contributors information.\n"
              << "  --usage           Show usage information.\n";
}