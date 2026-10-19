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

#include <classfile/SHA256.hpp>

std::string sha256(const std::filesystem::path& fullPath)
{
    std::ifstream file(fullPath, std::ios::binary);

    if (!file)
    {
        throw std::runtime_error("Failed to open file: " + fullPath.string());
    }

    using evp_md_ctx_ptr = std::unique_ptr<EVP_MD_CTX, decltype(&EVP_MD_CTX_free)>;

    evp_md_ctx_ptr context(EVP_MD_CTX_new(), EVP_MD_CTX_free);

    if (!context)
    {
        throw std::runtime_error("Failed to allocate SHA-256 context");
    }

    if (EVP_DigestInit_ex(context.get(), EVP_sha256(), nullptr) != 1)
    {
        throw std::runtime_error("Failed to initialise SHA-256");
    }

    std::array<char, 8192> buffer{};

    while (file)
    {
        file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));
        const auto bytesRead = file.gcount();

        if (bytesRead > 0 && EVP_DigestUpdate(context.get(), buffer.data(), static_cast<std::size_t>(bytesRead)) != 1)
        {
            throw std::runtime_error("Failed to update SHA-256 digest");
        }
    }

    if (!file.eof())
    {
        throw std::runtime_error("Failed to read file: " + fullPath.string());
    }

    std::array<unsigned char, EVP_MAX_MD_SIZE> digest{};
    unsigned int digestLen = 0;

    if (EVP_DigestFinal_ex(context.get(), digest.data(), &digestLen) != 1)
    {
        throw std::runtime_error("Failed to finalise SHA-256 digest");
    }

    std::ostringstream result;
    result << std::hex << std::setfill('0');

    for (unsigned int i = 0; i < digestLen; ++i)
    {
        result << std::setw(2) << static_cast<unsigned int>(digest[i]);
    }

    return result.str();
}
