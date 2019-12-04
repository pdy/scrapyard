#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

class GdalMess : public Application
{
public:
  GdalMess(int argc, char *argv[]);

protected:
  int main() override;
};


GdalMess::GdalMess(int argc, char *argv[]):
  Application(argc, argv, "GdalMess")
{ 
  Application::showHelpIfNoArguments(); 
  //Application::addCmdOption("option,o", "example of cmd option");
  Application::addCmdOption("file,f", "gdal dataset");
}

int GdalMess::main()
{
  LOG_INF << "GdalMess"; 

  const auto fileName = Application::getCmdOptionValue("file");

  GDALDataset  *poDataset;
  GDALAllRegister();
  poDataset = (GDALDataset *) GDALOpen( fileName.c_str(), GA_ReadOnly );
  if(!poDataset)
  {
    LOG_ERR << "poDataset null";
    return 1;
  }

  LOG_INF << "Got dataset!";

  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<GdalMess>(argc, argv);
}
