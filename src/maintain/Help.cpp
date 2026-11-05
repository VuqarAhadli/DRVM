/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "maintain/Help.hpp"
#include "maintain/ProgramInfo.hpp"

void printHelp()
{
    std::cout << "DRVM is a small JVM and java class file disassembler mainly designed for reverse engineering.\n"
              << "Version: " << DRVM_VERSION << "\n"
              << "Source code: https://github.com/VuqarAhadli/DRVM\n";
    printUsage();
    std::cout << "Options:\n"
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