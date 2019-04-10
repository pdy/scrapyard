#ifndef SEVERALGH_APPLICATION_H_
#define SEVERALGH_APPLICATION_H_

#include <string>
#include <vector>
#include <memory>
#include <type_traits>

#include <boost/program_options.hpp>

class Application
{
public:
    virtual ~Application() = default;

    int run();

    Application() = delete;
    Application(const Application&) = delete;
    Application(Application&&) = delete;
    Application& operator= (const Application&) = delete;
    Application& operator= (Application&&) = delete;

protected:
    Application(int argc, char *argv[]);
    Application(int argc, char *argv[], const std::string &appDescription);

    virtual bool init();
    virtual int main();

    std::string getAppName() const;

    bool addCmdOption(const std::string &option, const std::string &desc, bool required = true);
    bool addCmdOptionFlag(const std::string &option, const std::string &desc, bool defaultValue = false);

    bool getCmdOptionFlag(const std::string &option) const;
    std::string getCmdOptionValue(const std::string &option) const;	
 
    inline void showHelpIfNoArguments()
    {
        m_showHelpIfNoArguments = true;
    }
	
private:		
    const int m_argc;
    char **m_argv;

    void processArguments(int argc, char *argv[]);
    bool helpRequested() const;
    bool configureLogging();	
	
    std::unique_ptr<boost::program_options::variables_map> m_cmdArguments;
    std::unique_ptr<boost::program_options::options_description> m_cmdDesc;
    std::string m_appName;

    bool m_showHelpIfNoArguments = false;
};

namespace MainApplication{

    template
    <
        typename AppClass,
        typename = typename std::enable_if<std::is_base_of<Application, AppClass>::value>::type
    >
    int run(int argc, char *argv[])
    {
        auto app = std::make_unique<AppClass>(argc, argv);
        return app->run();	
    }

} // namespace MainApplication

#endif //SEVERALGH_APPLICATION_H_
