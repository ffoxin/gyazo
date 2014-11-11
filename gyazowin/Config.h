#pragma once

#include <fstream>
#include <map>
#include <string>

namespace Gyazo
{

class Section
{
public:
    Section();
    ~Section();

    void ParseParam(std::string const& line);
    std::string const& GetParam(std::string const& paramName) const;

private:
    typedef std::map < std::string, std::string > SectionData;
    SectionData m_params;
};

class Config
{
public:
    Config(std::wstring const& configFileName);
    ~Config();

    Section const& GetSection(std::string const& section) const;

private:
    typedef std::map < std::string, Section > ConfigData;
    ConfigData m_configData;
    std::ifstream m_configFile;
};

} // namespace Gyazo
