#include "Config.h"

namespace
{

inline std::string Substring(std::string const& src, std::size_t const begin, std::size_t const end)
{
    return src.substr(begin, end - begin);
}

} // namespace 

namespace Gyazo
{

Section::Section()
{
}

Section::~Section()
{
}

void Section::ParseParam(std::string const& line)
{
    std::size_t const delimPosition = line.find('=');
    std::string const paramName = Substring(line, 0, delimPosition);
    std::string const paramValue = Substring(line, delimPosition + 1, line.size());

    m_params.insert(std::make_pair(paramName, paramValue));
}

std::string const& Section::GetParam(std::string const& paramName) const
{
    SectionData::const_iterator it = m_params.find(paramName);
    if (it == m_params.end())
    {
        std::string const errorMessage = "Section::GetParam: parameter '" + paramName + "' not found";
        throw std::exception(errorMessage.c_str());
    }

    return it->second;
}

Config::Config(std::wstring const& configFileName) :
    m_configFile(configFileName)
{
    ConfigData::iterator currentSection = m_configData.end();
    while (!m_configFile.eof())
    {
        std::string line;
        std::getline(m_configFile, line);
        if (line.size())
        {
            if (line[0] == '[')
            {
                // start new section
                std::string const sectionName = Substring(line, 1, line.size() - 1);
                std::pair<ConfigData::iterator, bool> it = m_configData.insert(std::make_pair(sectionName, Section()));
                if (!it.second)
                {
                    std::string const errorMessage = "Config::Config: section '" + sectionName + "' already added to config";
                    throw std::exception(errorMessage.c_str());
                }
                currentSection = it.first;
            }
            else
            {
                currentSection->second.ParseParam(line);
            }
        }
    }
}

Config::~Config()
{
}

Section const& Config::GetSection(std::string const& section) const
{
    ConfigData::const_iterator it = m_configData.find(section);
    if (it == m_configData.end())
    {
        std::string const errorMessage = "Config::GetSection: section '" + section + "' not found";
        throw std::exception(errorMessage.c_str());
    }

    return it->second;
}

} // namespace Gyazo
