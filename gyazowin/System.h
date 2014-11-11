#pragma once

#include <string>

namespace Gyazo
{

class System
{
public:
    static std::wstring const& GetIdDirPath();

private:
    static std::wstring idDirPath;
};

} // namespace Gyazo
