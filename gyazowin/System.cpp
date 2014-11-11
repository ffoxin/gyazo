#include "System.h"

#include <Shlobj.h>

namespace Gyazo
{

std::wstring System::idDirPath;

std::wstring const& System::GetIdDirPath()
{
    if (idDirPath.length() == 0)
    {
        PWSTR idDir;
        HRESULT result = ::SHGetKnownFolderPath(
            FOLDERID_RoamingAppData,
            0,
            NULL,
            &idDir);

        if (result != S_OK)
        {
            throw std::exception("SHGetKnownFolderPath", result);
        }

        idDirPath = idDir;
        ::CoTaskMemFree(idDir);
    }

    return idDirPath;
}

} // namespace Gyazo
