#include "Convertion.h"

#include <sstream>

namespace Gyazo
{

int StringToInt(std::string const& str)
{
    std::istringstream ss(str);
    int value = 0;
    ss >> value;

    return value;
}

int WstringToInt(std::wstring const& wstr)
{
    std::wistringstream wss(wstr);
    int value = 0;
    wss >> value;

    return value;
}

} // namespace Gyazo
