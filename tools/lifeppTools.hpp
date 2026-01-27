#pragma once
#include <optional>
#include <filesystem>

namespace lifepp::tools{
    std::optional<std::filesystem::path> GetEnv(const std::string& name){
        const char* appdata = std::getenv(name.c_str());
        if(appdata){
            return std::filesystem::path(appdata);
        }
        return std::nullopt;
    }
}