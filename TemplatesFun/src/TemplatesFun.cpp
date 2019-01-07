#include "application/Application.h"
#include "application/TrivialLogger.h"

#include "EndiannessSwapper.h"

class TemplatesFun : public Application
{
public:
    TemplatesFun(int argc, char *argv[]);
    ~TemplatesFun() override = default;

protected:
    int main() override;
};


TemplatesFun::TemplatesFun(int argc, char *argv[])
    : Application(argc, argv, "TemplatesFun")
{
//    Application::showHelpIfNoArguments(); 
//    Application::addCmdOption("option,o", "example of cmd option");
//    Application::addCmdOptionFlag("flag,f", "example of cmd flag");
}


template<typename T>
void func(T&& t);

template<>
void func(int&)
{
    LOG_DBG_TRIV << "lvalue";
}

template<>
void func(int&&)
{
    LOG_DBG_TRIV << "rvalue";
}


template<typename T>
void func2(T t);

template<>
void func2(int)
{
    LOG_DBG_TRIV << "val";
}

template<>
void func2(int*)
{
    LOG_DBG_TRIV << "ptr";
}


template<class Derived>
class Interface
{
    const Derived& derived() const { return *static_cast<const Derived*>(this); }

public:
    std::string name() const { return derived().name(); }

    void run() const
    {
        derived().handleColor();
        derived().handleShape();
    }

    void handleColor() const { LOG_DBG_TRIV << "Interface handleColor"; }

    void handleShape() const { LOG_DBG_TRIV << "Interface handleShape"; }

private:
    Interface() = default;
    friend Derived;
};

class Derived : public Interface<Derived>
{
public:
    std::string name() const { return "Derived"; }

    void handleShape() const { LOG_DBG_TRIV << "Derived handleColor"; }
};

class Derived2 : public Interface<Derived2>
{
public:
    std::string name() const { return "Derived2"; }
};

template<class T>
void logName(const Interface<T> &obj)
{
    LOG_DBG_TRIV << obj.name();
}

int TemplatesFun::main()
{
    /*
    int x = 27;
    int& xr = x;
    func(x);
    func(27);

    func2(&x);
    func2(x);
    func2(xr);
*/
    Derived derived;
    logName(derived); 
    derived.run();
   
    Derived2 d2;
    logName(d2);
    d2.run();
    
    int tmp;
    float tmp2;
    templfunc(tmp);

    runTest();

    return 0;
}


int main(int argc, char *argv[])
{
    return MainApplication::run<TemplatesFun>(argc, argv);
}
