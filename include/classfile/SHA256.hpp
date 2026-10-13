#pragma once

#include <filesystem>
#include <string>
#include <array>
#include <fstream>
#include <iomanip>
#include <memory>
#include <sstream>
#include <stdexcept>

#include <openssl/evp.h>

std::string sha256(const std::filesystem::path& fullPath);
