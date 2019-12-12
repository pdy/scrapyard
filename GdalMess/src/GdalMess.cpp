#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <iostream>

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <gdal/gdalwarper.h>

#include "ScapsHeader.h"
#include "GeoTransform.h"


class GdalMess : public Application
{
public:
  GdalMess(int argc, char *argv[]);

protected:
  bool configureLogging() override;
  int main() override;
};


GDALDataset* getDataSet(const std::string &fileName, GDALAccess access = GDALAccess::GA_ReadOnly)
{
  //GDALDataset  *dataset;
  return (GDALDataset *) GDALOpen( fileName.c_str(), access );
}

bool copyCoordinateSystem(GDALDataset *srcDataset, GDALDataset *destDataset)
{
  const char* srcProj = srcDataset->GetProjectionRef();
  if(!srcProj)
  {
    LOG_ERR << "SRC proj null";
    return false;
  }

  /*
  double srcGeotransform[6];
  if( srcDataset->GetGeoTransform( srcGeotransform ) != CE_None )
  {
    LOG_ERR << "Cant get SRC geo transform";
    return false;
  }
 
  
  if(destDataset->SetGeoTransform(srcGeotransform) != CE_None)
  {
    LOG_ERR << "Cant SET geo transform";
    return false;
  }
  */

  if(destDataset->SetProjection(srcProj) != CE_None)
  {
    LOG_ERR << "Cant SET projection";
    return false;
  }

  if(const auto gcpCount {srcDataset->GetGCPCount()})
  {
    destDataset->SetGCPs(gcpCount, srcDataset->GetGCPs(), srcDataset->GetGCPProjection());
    return false;
  }

  return true;

}

GdalMess::GdalMess(int argc, char *argv[]):
  Application(argc, argv, "GdalMess")
{ 
  Application::showHelpIfNoArguments(); 
  Application::addCmdOption("in,i", "Source dataset file.");
  Application::addCmdOption("out,o", "File to be reprojected.");
}

bool GdalMess::configureLogging()
{
  return true;
}

int GdalMess::main()
{
  LOG_INF << "GdalMess"; 

  const auto source = Application::getCmdOptionValue("in");
  const auto dest = Application::getCmdOptionValue("out");

  GDALAllRegister();

  auto *srcDataset = getDataSet(source);
  if(!srcDataset)
  {
    LOG_ERR << "SRC dataset null";
    return 1;
  }

  auto *destDataset = getDataSet(dest, GDALAccess::GA_Update);
  if(!destDataset)
  {
    LOG_ERR << "DEST dataset null";
    return 1;
  }

  if(copyCoordinateSystem(srcDataset, destDataset))
  {
    LOG_INF << "Copy georef succeded";
  }
  
  AffineGeoTransform srcGeotransform(*srcDataset);  
  LOG_DBG << source << " geo transform:";
  srcGeotransform.prettyPrint();
  const auto pixCoord = srcGeotransform.getPixCoordinates(107, 67);
 
  const auto destRecalulatedAffineTransformation = srcGeotransform.recalcOrigo({223, 373}, pixCoord);
  if(!destRecalulatedAffineTransformation.copyToDataset(*destDataset))
  {
    LOG_ERR << "Copy to dest failed";
    return 1;
  }

  LOG_DBG << dest << " geo transform:";
  destRecalulatedAffineTransformation.prettyPrint();
  LOG_DBG << "";

  //srcGeotransform.upperLeftPixPosition().prettyPrint();

  
  

  GDALClose(srcDataset);
  GDALClose(destDataset);
  
  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<GdalMess>(argc, argv);
}
