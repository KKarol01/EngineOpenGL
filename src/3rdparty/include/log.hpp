#pragma once

#include <iostream>

void _log(const char *file_name, const char *func_name, uint32_t line_num, const auto &arg1, const auto &...args) {
    printf("(%s, %s, %u): \"", file_name, func_name, line_num);
    std::cout << arg1;
    ([](const auto &p) { std::cout << ", " << p; }(args), ...);
    std::cout << "\"\n";
}

#define __FILENAME__ strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__
#define LOG(...) _log(__FILENAME__, __func__, __LINE__, __VA_ARGS__);