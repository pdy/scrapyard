#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <iostream>

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <gdal/gdalwarper.h>

class GdalMess : public Application
{
public:
  GdalMess(int argc, char *argv[]);

protected:
  int main() override;
};

/*
void datasetInfo(GDALDataset *dataset)
{
  double        adfGeoTransform[6];
  printf( "Driver: %s/%s\n",
        dataset->GetDriver()->GetDescription(),
        dataset->GetDriver()->GetMetadataItem( GDAL_DMD_LONGNAME ) );
  printf( "Size is %dx%dx%d\n",
        dataset->GetRasterXSize(), dataset->GetRasterYSize(),
        dataset->GetRasterCount() );
  if( dataset->GetProjectionRef()  != NULL )
    printf( "Projection is `%s'\n", dataset->GetProjectionRef() );
  if( dataset->GetGeoTransform( adfGeoTransform ) == CE_None )
  {
    printf( "Origin = (%.6f,%.6f)\n",
            adfGeoTransform[0], adfGeoTransform[3] );
    printf( "Pixel Size = (%.6f,%.6f)\n",
            adfGeoTransform[1], adfGeoTransform[5] );
  }

}
*/
/*
void fetchBand(GDALDataset *dataset)
{
  GDALRasterBand *band;
int             nBlockXSize, nBlockYSize;
int             bGotMin, bGotMax;
double          adfMinMax[2];
band = dataset->GetRasterBand( 1 );
band->GetBlockSize( &nBlockXSize, &nBlockYSize );
printf( "Block=%dx%d Type=%s, ColorInterp=%s\n",
        nBlockXSize, nBlockYSize,
        GDALGetDataTypeName(band->GetRasterDataType()),
        GDALGetColorInterpretationName(
            band->GetColorInterpretation()) );
adfMinMax[0] = band->GetMinimum( &bGotMin );
adfMinMax[1] = band->GetMaximum( &bGotMax );
if( ! (bGotMin && bGotMax) )
    GDALComputeRasterMinMax((GDALRasterBandH)band, TRUE, adfMinMax);
printf( "Min=%.3fd, Max=%.3f\n", adfMinMax[0], adfMinMax[1] );
if( band->GetOverviewCount() > 0 )
    printf( "Band has %d overviews.\n", band->GetOverviewCount() );
if( band->GetColorTable() != NULL )
    printf( "Band has a color table with %d entries.\n",
            band->GetColorTable()->GetColorEntryCount() );
  
}
*/


GDALDataset* getDataSet(const std::string &fileName, GDALAccess access = GDALAccess::GA_ReadOnly)
{
  GDALDataset  *dataset;
  return (GDALDataset *) GDALOpen( fileName.c_str(), access );
}

bool copyGeoref(GDALDataset *srcDataset, GDALDataset *destDataset)
{
  const char* srcProj = srcDataset->GetProjectionRef();
  if(!srcProj)
  {
    LOG_ERR << "SRC proj null";
    return false;
  }

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

  if(copyGeoref(srcDataset, destDataset))
  {
    LOG_INF << "Copy georef succeded";
  }
  

  GDALClose(srcDataset);
  GDALClose(destDataset);


  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<GdalMess>(argc, argv);
}
