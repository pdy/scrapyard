#include "application/Application.h"
#include "application/TrivialLogger.h"

class TaggedUnionTests : public Application
{
public:
  TaggedUnionTests(int argc, char *argv[]);

protected:
  int main() override;
};


TaggedUnionTests::TaggedUnionTests(int argc, char *argv[]):
  Application(argc, argv, "TaggedUnionTests")
{ 
  //Application::showHelpIfNoArguments(); 
  //Application::addCmdOption("option,o", "example of cmd option");
  //Application::addCmdOptionFlag("flag,f", "example of cmd flag");
}

template<class T>
struct MyType
{
  using type = T;
  union {
    int i;
    float f;
  };
};

template<class T>
bool isInt(const MyType<T>&) { return false;}

template<>
bool isInt(const MyType<int>&) { return true;}

int TaggedUnionTests::main()
{
  LOG_INF << "TaggedUnionTests";

  MyType<int> mytype;
  mytype.i = 30;
  LOG_INF << "int " << mytype.i << " is int " << isInt(mytype);

  MyType<float> myf;
  myf.f = 30.5;
  LOG_INF << "float " << myf.f << " is int " << isInt(myf);

  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<TaggedUnionTests>(argc, argv);
}
