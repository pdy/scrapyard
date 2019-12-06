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

class GeoTransform
{
public:
  struct ProjectionCoordinates
  {
    double Xp;
    double Yp;
  };

  GeoTransform(GDALDataset &dataset)
  {
    if( dataset.GetGeoTransform(m_gdalGeotransform) != CE_None )
    {
      LOG_ERR << "Cant get SRC geo transform";
      throw "";
    }
  } 

  double pixelWidth() const { return m_gdalGeotransform[1]; }
  double pixelHeight() const { return m_gdalGeotransform[5]; }
  double upperLeftPixX() const { return m_gdalGeotransform[0]; }
  double upperLeftPixY() const { return m_gdalGeotransform[3]; }

  void prettyPrint() const
  {
    LOG_DBG << "  pixel width [" << pixelWidth() << "]";
    LOG_DBG << "  pixel height [" << pixelHeight() << "]";
    LOG_DBG << "  upper left pixel X [" << upperLeftPixX() << "]";
    LOG_DBG << "  upper left pixel Y [" << upperLeftPixY() << "]";
  }

  ProjectionCoordinates upperLeftPixPosition() const
  {
    return { upperLeftPixX(), upperLeftPixY() };
  }

  ProjectionCoordinates pixPosition(double pixel, double line) const
  {
    const double Xp = upperLeftPixX() + pixel * pixelWidth() + line * m_gdalGeotransform[2];
    const double Yp = upperLeftPixY() + pixel * m_gdalGeotransform[4] + line * pixelHeight();

    return { Xp, Yp };
  }

private:
  double m_gdalGeotransform[6];

};

GDALDataset* getDataSet(const std::string &fileName, GDALAccess access = GDALAccess::GA_ReadOnly)
{
  GDALDataset  *dataset;
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

  double srcGeotransform[6];
  if( srcDataset->GetGeoTransform( srcGeotransform ) != CE_None )
  {
    LOG_ERR << "Cant get SRC geo transform";
    return false;
  }
 
  /*
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
  
  GeoTransform srcGeotransform(*srcDataset);  
  LOG_DBG << source << " geo transform:";
  srcGeotransform.prettyPrint();
  LOG_DBG << "";

  GeoTransform destGeotransform(*destDataset);  
  LOG_DBG << dest << " geo transform:";
  destGeotransform.prettyPrint();
  LOG_DBG << "";

  GDALClose(srcDataset);
  GDALClose(destDataset);


  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<GdalMess>(argc, argv);
}
