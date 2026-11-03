
#pragma once

#include <cstdint>
#include <string>
#include <stdexcept>

constexpr uint64_t SCALE = 100;

inline uint64_t toFixed(const std::string& input) {

    size_t pos = input.find('.');

    std::string whole = (pos == std::string::npos)
                        ? input
                        : input.substr(0, pos);

    std::string decimal = (pos == std::string::npos)
                          ? ""
                          : input.substr(pos + 1);

    if (decimal.size() > 2) {
        throw std::invalid_argument("Maximum 2 decimal places allowed");
    }

    while (decimal.size() < 2) {
        decimal += '0';
    }

    uint64_t integerPart = std::stoull(whole);
    uint64_t decimalPart = decimal.empty()
                           ? 0
                           : std::stoull(decimal);

    return integerPart * SCALE + decimalPart;
}

inline std::string fromFixed(uint64_t value) {

    return std::to_string(value / SCALE) + "." +
           (value % SCALE < 10 ? "0" : "") +
           std::to_string(value % SCALE);
}
