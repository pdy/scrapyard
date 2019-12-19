#include "application/Application.h"
#include "application/TrivialLogger.h"

#include "maybe.hpp"

class MaybeTests : public Application
{
public:
  MaybeTests(int argc, char *argv[]);

protected:
  int main() override;
};


MaybeTests::MaybeTests(int argc, char *argv[]):
  Application(argc, argv, "MaybeTests")
{ 
  //Application::showHelpIfNoArguments(); 
  //Application::addCmdOption("option,o", "example of cmd option");
  //Application::addCmdOptionFlag("flag,f", "example of cmd flag");
}

struct Type {};

inline neither::Maybe<int> funcInt()
{
  return neither::Maybe<int>(12);
}

inline neither::Maybe<std::unique_ptr<Type>> funcUptr()
{
  auto tmp = std::make_unique<Type>();
  return neither::Maybe<std::unique_ptr<Type>>(std::move(tmp));
}

int MaybeTests::main()
{
  LOG_INF << "MaybeTests";
  funcInt();
  funcUptr(); 
  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<MaybeTests>(argc, argv);
}
