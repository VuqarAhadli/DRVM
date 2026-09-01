#!/bin/bash
# DRVM (drvm)
# Copyright (C) 2026 Vugar Ahadli
# Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program. If not, see <https://www.gnu.org/licenses/>.
#
# SPDX-License-Identifier: GPL-3.0-or-later

set -e

if [ ! -d "build" ]; then
    mkdir build
fi


cd ./build
echo "Building project..."
cmake ..
make 

echo ""
echo "Starting Benchmark"
echo "=&$@#$%^&!@#$%(=&$@#$%^&!@#$%("
echo ""

echo "#&=*/!  DRVM - 50 runs  #&=*/!"
/usr/bin/time -l sh -c 'for i in {1..50}; do ./drvm ../extracted/i.class > /dev/null; done'

echo ""
echo "#&=*/!  JAVAP - 50 runs  #&=*/!"
/usr/bin/time -l sh -c 'for i in {1..50}; do javap -v ../extracted/i.class > /dev/null; done'

echo ""
echo "=&$@#$%^&!@#$%(=&$@#$%^&!@#$%("
echo "Done!"