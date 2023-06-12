#pragma once
#include <string>
#include <format>
#include <Windows.h>
inline std::wstring ConvertString(const std::string& str);

std::string ConvertString(const std::wstring& str);