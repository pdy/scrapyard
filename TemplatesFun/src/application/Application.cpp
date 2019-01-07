#include "Application.h"
#include "TrivialLogger.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <errno.h>

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
    :   _argc(argc),
        _argv(argv),
        _cmdArguments(new options::variables_map()),
        _cmdDesc(new options::options_description("Options"))
      
{ 
    _cmdDesc->add_options()("help,h","Print help message"); 
}

Application::Application(int argc, char *argv[], const std::string &appDesc)
    :   _argc(argc),
        _argv(argv),
        _cmdArguments(new options::variables_map()),
        _cmdDesc(new options::options_description(appDesc))
{
	_cmdDesc->add_options()("help,h","Print help message");
}

std::string Application::getAppName() const
{
    return _appName;
}

int Application::run()
{
    try
    {
        if(!init())
        {
            LOG_ERR_TRIV << "Initialization failed";
            return -1;
        }

        processArguments(_argc, _argv);

        if(helpRequested())
        {
            std::cout << *_cmdDesc << "\n";
            return 0;
        }

        options::notify(*_cmdArguments); 
    }
    catch(const options::error &e)
    {
        LOG_ERR_TRIV << "ERROR: " << e.what();
        return -1;
    }
    catch(const std::exception &e)
    {
        LOG_ERR_TRIV << "Exception, thrown [" << e.what() << "]";
        return -1;
    }
    catch(...)
    {
        LOG_ERR_TRIV << "Unknown exception thrown";
        return -1;
    }
	
    return main();
}

void Application::processArguments(int argc, char *argv[])
{	
    boost::filesystem::path p = argv[0];	
    _appName = p.stem().string();

    options::variables_map vm; 
    options::store(options::command_line_parser(argc, argv).options(* _cmdDesc).run(), vm);
	    
    _cmdArguments.reset(new options::variables_map(vm));	
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
    return _cmdArguments->count("help") || (_showHelpIfNoArguments && _argc == 1);
}

bool Application::getCmdOptionFlag(const std::string &option) const
{
    return (*_cmdArguments.get())[option].as<bool>();
}

bool Application::addCmdOption(const std::string &option, const std::string &desc, bool required)
{    
    const auto found = std::find_if(_cmdDesc->options().cbegin(), _cmdDesc->options().cend(), 
            [&option](const DescriptionPtr &entry){  
                return entry->match(getLong(option), true, true, true) != options::option_description::no_match
                    || entry->match(getShort(option), true, true, true) != options::option_description::no_match;
            });
    
    const bool unique = found == _cmdDesc->options().cend();
	
    if(unique && required)
    {
        _cmdDesc->add_options()
            (option.c_str(), options::value<std::string>()->required(), desc.c_str());
    }
	else if(unique)
    {
        _cmdDesc->add_options()
            (option.c_str(),options::value<std::string>(),desc.c_str());
    }
    else
    {
        LOG_WRN_TRIV << "Option " << option << " is already taken";
    }

    return unique;
}

bool Application::addCmdOptionFlag(const std::string &option, const std::string &desc, bool defaultValue)
{
    const auto found = std::find_if(_cmdDesc->options().cbegin(), _cmdDesc->options().cend(), 
            [&option](const DescriptionPtr &entry){  
                return entry->match(getLong(option), true, true, true) != options::option_description::no_match
                    || entry->match(getShort(option), true, true, true) != options::option_description::no_match;
            });
    
    const bool unique = found == _cmdDesc->options().cend();

    if(unique)
    {
        _cmdDesc->add_options()
            (option.c_str(), options::bool_switch()->default_value(defaultValue), desc.c_str());
    }
    else
    {
        LOG_WRN_TRIV << "Option " << option << " is already taken";
    }

    return unique;
}

std::string Application::getCmdOptionValue(const std::string &option) const
{
    if(_cmdArguments->count(option))
    {
        return (*_cmdArguments)[option].as<std::string>();
    }

    return "";
}

int Application::main()
{
    LOG_INF_TRIV << "main(): I should be reimplemented dummy";
    return 0;
}
