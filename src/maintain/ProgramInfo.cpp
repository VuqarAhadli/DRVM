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

#include "maintain/ProgramInfo.hpp"

void printVersion()
{
    std::cout << "DRVM Version: " << DRVM_VERSION << '\n';
}

void printLicense()
{
    std::cout << "DRVM is licensed under the GNU General Public License v3.0 (GPLv3).\n"
              << "You can find the full license text at: https://www.gnu.org/licenses/gpl-3.0.en.html\n"
              << "or in the LICENSE file distributed with the source code.\n";
}

void printContributors()
{
    std::cout << "Author: Vugar Ahadli\n"
              << "\tMain developer and maintainer of the project.\n"
              << "\tE-mail: vuqar@div.edu.az\n"
              << "\tGitHub: VuqarAhadli\n"
              << "\n"
              << "Contributor: Azer Yusifzade\n"
              << "\tMaintainer of the Windows development subsystem.\n"
              << "\tE-mail: azeryusifzade29@gmail.com\n"
              << "\tGitHub: azeryusifzade\n";
}

void printUsage()
{
    std::cout << "Usage: drvm <class file> [--dump] [--run] [--trace] [--debug]\n";
}

void printInfo()
{
    printVersion();
    printLicense();
    printContributors();
    printUsage();
}
