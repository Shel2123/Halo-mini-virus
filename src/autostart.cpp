#include "autostart.h"

#include "app_constants.h"
#include "win32_common.h"

#include <cwchar>
#include <filesystem>
#include <fstream>
#include <string>

#include <knownfolders.h>
#include <objbase.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shlwapi.h>

namespace fs = std::filesystem;

namespace halo
{
    namespace
    {
        std::wstring GetKnownFolderPath(REFKNOWNFOLDERID id)
        {
            PWSTR buffer = nullptr;
            std::wstring result;
            if (SUCCEEDED(SHGetKnownFolderPath(id, KF_FLAG_CREATE, nullptr, &buffer)))
            {
                result.assign(buffer);
                CoTaskMemFree(buffer);
            }
            return result;
        }

        std::wstring GetAppDataRoaming()
        {
            return GetKnownFolderPath(FOLDERID_RoamingAppData);
        }

        std::wstring GetStartupFolder()
        {
            return GetKnownFolderPath(FOLDERID_Startup);
        }

        std::wstring GetThisExecutablePath()
        {
            wchar_t buffer[MAX_PATH]{};
            GetModuleFileNameW(nullptr, buffer, MAX_PATH);
            return std::wstring(buffer);
        }

        bool FileExists(const std::wstring &path)
        {
            return PathFileExistsW(path.c_str()) == TRUE;
        }

        bool CopySelfTo(const std::wstring &destination)
        {
            const std::wstring source = GetThisExecutablePath();
            if (_wcsicmp(source.c_str(), destination.c_str()) == 0)
            {
                return true;
            }

            try
            {
                fs::create_directories(fs::path(destination).parent_path());
            }
            catch (...)
            {
            }

            return CopyFileW(source.c_str(), destination.c_str(), FALSE) == TRUE;
        }

        bool CreateShellLink(const std::wstring &targetPath, const std::wstring &linkPath)
        {
            HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
            const bool didInit = (hr == S_OK || hr == S_FALSE);

            IShellLinkW *shellLink = nullptr;
            IPersistFile *persistFile = nullptr;
            bool success = false;

            do
            {
                hr = CoCreateInstance(CLSID_ShellLink, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&shellLink));
                if (FAILED(hr) || shellLink == nullptr)
                {
                    break;
                }

                if (FAILED(shellLink->SetPath(targetPath.c_str())))
                {
                    break;
                }

                const std::wstring workdir = fs::path(targetPath).parent_path().wstring();
                shellLink->SetWorkingDirectory(workdir.c_str());
                shellLink->SetDescription(L"Halo autostart");

                hr = shellLink->QueryInterface(IID_PPV_ARGS(&persistFile));
                if (FAILED(hr) || persistFile == nullptr)
                {
                    break;
                }

                hr = persistFile->Save(linkPath.c_str(), TRUE);
                if (FAILED(hr))
                {
                    break;
                }

                success = true;
            } while (false);

            if (persistFile)
            {
                persistFile->Release();
            }
            if (shellLink)
            {
                shellLink->Release();
            }
            if (didInit)
            {
                CoUninitialize();
            }

            return success;
        }

        bool AddToRunRegistry(const std::wstring &name, const std::wstring &exePath)
        {
            HKEY key = nullptr;
            const LONG openResult = RegOpenKeyExW(
                HKEY_CURRENT_USER,
                L"Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                0,
                KEY_SET_VALUE,
                &key);
            if (openResult != ERROR_SUCCESS)
            {
                return false;
            }

            const std::wstring value = L"\"" + exePath + L"\"";
            const LONG setResult = RegSetValueExW(
                key,
                name.c_str(),
                0,
                REG_SZ,
                reinterpret_cast<const BYTE *>(value.c_str()),
                static_cast<DWORD>((value.size() + 1) * sizeof(wchar_t)));
            RegCloseKey(key);
            return setResult == ERROR_SUCCESS;
        }
    } // namespace

    void EnsureAutostartInstalled()
    {
        const std::wstring appData = GetAppDataRoaming();
        if (appData.empty())
        {
            return;
        }

        const fs::path installDir = fs::path(appData) / kAppFolderName;
        const fs::path destinationExe = installDir / kAppFileName;
        const fs::path installedFlag = installDir / kInstalledFlagName;

        if (fs::exists(installedFlag))
        {
            return;
        }

        const bool copied = CopySelfTo(destinationExe.wstring());
        const std::wstring targetExe = copied ? destinationExe.wstring() : GetThisExecutablePath();

        bool autostartReady = false;

        const std::wstring startup = GetStartupFolder();
        if (!startup.empty())
        {
            const fs::path linkPath = fs::path(startup) / kStartupLinkName;
            if (!FileExists(linkPath.wstring()))
            {
                autostartReady = CreateShellLink(targetExe, linkPath.wstring());
            }
            else
            {
                autostartReady = true;
            }
        }

        if (!autostartReady)
        {
            autostartReady = AddToRunRegistry(L"Halo", targetExe);
        }

        if (autostartReady)
        {
            try
            {
                fs::create_directories(installDir);
                std::ofstream flag(installedFlag);
                flag << "installed";
            }
            catch (...)
            {
            }
        }
    }
} // namespace halo
