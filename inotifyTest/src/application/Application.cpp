#include "Application.h"
#include "TrivialLogger.h"

#include <iostream>

#include <boost/filesystem.hpp>

namespace options = boost::program_options;
using DescriptionPtr = boost::shared_ptr<options::option_description>;

namespace {

std::string getLong(const std::string &option)
{
    const auto commaPos = option.find(',');
    if(commaPos == std::string::npos)
    {
        return option;
    }

    return option.substr(0, commaPos);
}

std::string getShort(const std::string &option)
{
    const auto commaPos = option.find(',');
    if(commaPos == std::string::npos)
    {
        return option;
    }

    return option.substr(commaPos + 1);
}

} // namespace

Application::Application(int argc, char *argv[])
    :   m_argc(argc),
        m_argv(argv),
        m_cmdArguments(new options::variables_map()),
        m_cmdDesc(new options::options_description("Options"))
      
{ 
    m_cmdDesc->add_options()("help,h","Print help message"); 
}

Application::Application(int argc, char *argv[], const std::string &appDesc)
    :   m_argc(argc),
        m_argv(argv),
        m_cmdArguments(new options::variables_map()),
        m_cmdDesc(new options::options_description(appDesc))
{
    m_cmdDesc->add_options()("help,h","Print help message");
}

std::string Application::getAppName() const
{
    return m_appName;
}

int Application::run()
{
    try
    {
        if(!init())
        {
            LOG_ERR << "Initialization failed";
            return -1;
        }

        processArguments(m_argc, m_argv);

        if(helpRequested())
        {
            std::cout << *m_cmdDesc << "\n";
            return 0;
        }

        options::notify(*m_cmdArguments); 
    }
    catch(const options::error &e)
    {
        LOG_ERR << "ERROR: " << e.what();
        return -1;
    }
    catch(const std::exception &e)
    {
        LOG_ERR << "Exception, thrown [" << e.what() << "]";
        return -1;
    }
    catch(...)
    {
        LOG_ERR << "Unknown exception thrown";
        return -1;
    }
	
    return main();
}

void Application::processArguments(int argc, char *argv[])
{	
    boost::filesystem::path p = argv[0];	
    m_appName = p.stem().string();

    options::variables_map vm; 
    options::store(options::command_line_parser(argc, argv).options(* m_cmdDesc).run(), vm);
	    
    m_cmdArguments.reset(new options::variables_map(vm));	
}

bool Application::init()
{	
    if(!configureLogging())
    {
        return false;
    }

    return true;
}

bool Application::configureLogging()
{
    return true;
}

bool Application::helpRequested() const
{
    return m_cmdArguments->count("help") || (m_showHelpIfNoArguments && m_argc == 1);
}

bool Application::getCmdOptionFlag(const std::string &option) const
{
    return (*m_cmdArguments.get())[option].as<bool>();
}

bool Application::addCmdOption(const std::string &option, const std::string &desc, bool required)
{    
    const auto found = std::find_if(m_cmdDesc->options().cbegin(), m_cmdDesc->options().cend(), 
            [&option](const DescriptionPtr &entry){  
                return entry->match(getLong(option), true, true, true) != options::option_description::no_match
                    || entry->match(getShort(option), true, true, true) != options::option_description::no_match;
            });
    
    const bool unique = found == m_cmdDesc->options().cend();
	
    if(unique && required)
    {
        m_cmdDesc->add_options()
            (option.c_str(), options::value<std::string>()->required(), desc.c_str());
    }
	else if(unique)
    {
        m_cmdDesc->add_options()
            (option.c_str(),options::value<std::string>(),desc.c_str());
    }
    else
    {
        LOG_WRN << "Option " << option << " is already taken";
    }

    return unique;
}

bool Application::addCmdOptionFlag(const std::string &option, const std::string &desc, bool defaultValue)
{
    const auto found = std::find_if(m_cmdDesc->options().cbegin(), m_cmdDesc->options().cend(), 
            [&option](const DescriptionPtr &entry){  
                return entry->match(getLong(option), true, true, true) != options::option_description::no_match
                    || entry->match(getShort(option), true, true, true) != options::option_description::no_match;
            });
    
    const bool unique = found == m_cmdDesc->options().cend();

    if(unique)
    {
        m_cmdDesc->add_options()
            (option.c_str(), options::bool_switch()->default_value(defaultValue), desc.c_str());
    }
    else
    {
        LOG_WRN << "Option " << option << " is already taken";
    }

    return unique;
}

std::string Application::getCmdOptionValue(const std::string &option) const
{
    if(m_cmdArguments->count(option))
    {
        return (*m_cmdArguments)[option].as<std::string>();
    }

    return "";
}

int Application::main()
{
    LOG_INF << "main(): I should be reimplemented dummy";
    return 0;
}
