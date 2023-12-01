
#include "stdafx.h"

#include "Helpers.h"

#include <fstream>
#include <filesystem>

namespace Helper
{
    Json::Value ParseJson(const std::string& filepath)
    {
        if (std::filesystem::exists(filepath))
        {
            std::ifstream file(filepath, std::ios_base::binary);

            // TODO: handle error / add logs

            Json::Value root;
            file >> root;

            return root;
        }
    }
}
