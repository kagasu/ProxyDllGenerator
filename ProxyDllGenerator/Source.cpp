#include <Windows.h>
#include <winnt.h>
#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <codecvt>
#include <vector>
#include <algorithm>
#include <list>
#include <filesystem>
#include <boost/range/irange.hpp>
#include <boost/range/adaptor/transformed.hpp>
#include <fmt/format.h>
#include <boost/program_options.hpp>
#include "ToVector.hpp"
#include "FunctionInfo.hpp"

#if _WIN32 || _WIN64
#if _WIN64
#define ENVIRONMENT64
#else
#define ENVIRONMENT32
#endif
#endif

#define DebugLog(str) std::cout << "[DEBUG] [LINE: " << std::dec << __LINE__ << "] " << str << std::endl;

std::string GenerateGuid()
{
    GUID guid;
    auto result = CoCreateGuid(&guid);
    if (result == S_OK)
    {
        return fmt::format("{:08x}{:04x}{:04x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}{:02x}",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
    }
    else
    {
        return std::string();
    }
}

std::string ReadAllText(const std::string& filePath)
{
    std::ifstream ifs(filePath, std::ios::in);
    std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
    ifs.close();

    return str;
}

void ReplaceString(std::string& str1, const std::string &str2, const std::string& str3)
{
    std::string::size_type Pos(str1.find(str2));

    while (Pos != std::string::npos)
    {
        str1.replace(Pos, str2.length(), str3);
        Pos = str1.find(str2, Pos + str3.length());
    }
}

// https://stackoverflow.com/a/3999597/4771485
std::string utf8_encode(const std::wstring& wstr)
{
    if (wstr.empty()) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// https://stackoverflow.com/a/3999597/4771485
std::wstring utf8_decode(const std::string& str)
{
    if (str.empty()) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

void generateProxyDllSourceCode32(const uintptr_t moduleAddress, int numberOfNames, const uint32_t addressOfNames[], const std::string& originalFileName, const std::string& outputDirectory)
{
    auto dllMainTemplate = ReadAllText("templates/32/DllMain.cpp");
    auto functionDefineTemplate = ReadAllText("templates/32/FunctionDefine.cpp");

    std::ofstream ofsDllMain(fmt::format("{0}/DllMain.cpp", outputDirectory), std::ios::out);
    ReplaceString(dllMainTemplate, "$FUNCTION_COUNT$", std::to_string(numberOfNames));

    auto functionInfos = boost::irange<int>(0, numberOfNames)
        | boost::adaptors::transformed([&moduleAddress, &addressOfNames](int i)
            {
                FunctionInfo fi(i, GenerateGuid(), std::string(reinterpret_cast<char*>(moduleAddress + addressOfNames[i])));
                return fi;
            })
        | boost_extension::to_vec;
    auto functionNames = functionInfos
        | boost::adaptors::transformed([](auto x)
            {
                return fmt::format("\"{0}\"", x.name);
            });

    auto functionDefines = functionInfos
        | boost::adaptors::transformed([&functionDefineTemplate](auto x)
            {
                return fmt::format(functionDefineTemplate, x.name, x.guid, x.index, x.index + 1);
            });

    ReplaceString(dllMainTemplate, "$FUNCTION_NAMES$", fmt::format("{0}", fmt::join(functionNames, ",\n\t")));
    ReplaceString(dllMainTemplate, "$FUNCTION_DEFINES$", fmt::format("{0}", fmt::join(functionDefines, "\n")));
    ReplaceString(dllMainTemplate, "$ORIGINAL_DLL_NAME$", fmt::format("\"{0}\"", originalFileName));
    ofsDllMain << dllMainTemplate;
    ofsDllMain.close();
}

void generateProxyDllSourceCode64(const uintptr_t moduleAddress, int numberOfNames, const uint32_t addressOfNames[], const std::string& originalFileName, const std::string& outputDirectory)
{
    auto dllMainTemplate = ReadAllText("templates/64/DllMain.cpp");
    auto functionDefineTemplate = ReadAllText("templates/64/FunctionDefine.cpp");
    auto asmTemplate = ReadAllText("templates/64/asm.asm");
    auto asmFunctionDefineTemplate = ReadAllText("templates/64/asmFunctionDefine.asm");

    std::ofstream ofsDllMain(fmt::format("{0}/DllMain.cpp", outputDirectory), std::ios::out);
    ReplaceString(dllMainTemplate, "$FUNCTION_COUNT$", std::to_string(numberOfNames));

    auto functionInfos = boost::irange<int>(0, numberOfNames)
        | boost::adaptors::transformed([&moduleAddress, &addressOfNames](int i)
            {
                FunctionInfo fi(i, GenerateGuid(), std::string(reinterpret_cast<char*>(moduleAddress + addressOfNames[i])));
                return fi;
            })
        | boost_extension::to_vec;
    auto functionNames = functionInfos
        | boost::adaptors::transformed([](auto x)
            {
                return fmt::format("\"{0}\"", x.name);
            });

    auto functionDefines = functionInfos
        | boost::adaptors::transformed([&functionDefineTemplate](auto x)
            {
                return fmt::format(functionDefineTemplate, x.name, x.guid, x.index, x.index + 1);
            });

    ReplaceString(dllMainTemplate, "$FUNCTION_NAMES$", fmt::format("{0}", fmt::join(functionNames, ",\n\t")));
    ReplaceString(dllMainTemplate, "$FUNCTION_DEFINES$", fmt::format("{0}", fmt::join(functionDefines, "\n")));
    ReplaceString(dllMainTemplate, "$ORIGINAL_DLL_NAME$", fmt::format("\"{0}\"", originalFileName));
    ofsDllMain << dllMainTemplate;
    ofsDllMain.close();

    std::ofstream ofsAsm(fmt::format("{0}/asm.asm", outputDirectory), std::ios::out);
    auto asmFunctionDefines = functionInfos
        | boost::adaptors::transformed([&asmFunctionDefineTemplate](auto x)
            {
                return fmt::format(asmFunctionDefineTemplate, x.name, x.guid, x.index, x.index + 1);
            });
    ReplaceString(asmTemplate, "$ASM_FUNCTION_DEFINES$", fmt::format("{0}", fmt::join(asmFunctionDefines, "\n")));
    ofsAsm << asmTemplate;
    ofsAsm.close();
}

int main(int argc, char** argv)
{
    boost::program_options::options_description description("ProxyDllGenerator");
    description.add_options()("dll,d", boost::program_options::value<std::string>(), "DLL file");

    boost::program_options::variables_map vm;
    try
    {
        boost::program_options::store(boost::program_options::parse_command_line(argc, argv, description), vm);
    }
    catch (const boost::program_options::error_with_option_name& e)
    {
        std::cout << e.what() << std::endl;
        return 0;
    }
    notify(vm);

    if (!vm.count("dll"))
    {
        std::cout << description << std::endl;
        return 0;
    }

    auto inputFileName = vm["dll"].as<std::string>();
    if (inputFileName.empty())
    {
        std::cout << description << std::endl;
        return 0;
    }
    auto originalFileName = std::filesystem::path(inputFileName).stem().generic_string() + "_original.dll";
    auto outputDirectory = std::string("out");

    if (!std::filesystem::exists(outputDirectory))
    {
        std::filesystem::create_directory(outputDirectory);
    }

    if (!std::filesystem::exists(inputFileName))
    {
        std::cout << "Input file not found" << std::endl;
        return 0;
    }

    auto moduleHandle = LoadLibraryExA(inputFileName.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
    if (moduleHandle == nullptr)
    {
        DebugLog("moduleHandle == nullptr");
        return -1;
    }
    auto moduleAddress = reinterpret_cast<uintptr_t>(moduleHandle);
    auto moduleDosHeader = reinterpret_cast<PIMAGE_DOS_HEADER>(moduleAddress);
    auto header = reinterpret_cast<PIMAGE_NT_HEADERS>(moduleAddress + moduleDosHeader->e_lfanew);
    auto exports = reinterpret_cast<PIMAGE_EXPORT_DIRECTORY>(moduleAddress + header->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    auto addressOfNames = reinterpret_cast<uint32_t*>(moduleAddress + exports->AddressOfNames);
    auto numberOfNames = static_cast<int>(exports->NumberOfNames);

#ifdef ENVIRONMENT64
    generateProxyDllSourceCode64(moduleAddress, numberOfNames, addressOfNames, originalFileName, outputDirectory);
#else
    generateProxyDllSourceCode32(moduleAddress, numberOfNames, addressOfNames, originalFileName, outputDirectory);
#endif
}
