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
    std::cout << "Usage: drvm <class file> [--dump] [--run] [--trace]\n";
}

void printInfo()
{
    printVersion();
    printLicense();
    printContributors();
    printUsage();
}