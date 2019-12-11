#include "application/Application.h"
#include "application/TrivialLogger.h"

#include <iostream>

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <gdal/gdalwarper.h>

#include "ScapsHeader.h"

class GdalMess : public Application
{
public:
  GdalMess(int argc, char *argv[]);

protected:
  int main() override;
};


class GeoTransform
{
public:
  struct PixelImagePosition
  {
    double dx {.0};
    double dy {.0};
  };
  struct PixelCoordinates
  {
    PixelImagePosition pixelImagePos;
    double Xp;
    double Yp;
    void prettyPrint() const
    {
      LOG_DBG << "Pixel " << pixelImagePos.dx << "x" << pixelImagePos.dy << " maps to coord:";
      LOG_DBG << "  Xp [" << Xp << "]";
      LOG_DBG << "  Yp [" << Yp << "]";
    }
  };

  GeoTransform(GDALDataset &dataset)
  {
    if( dataset.GetGeoTransform(m_gdalGeotransform) != CE_None )
    {
      LOG_ERR << "Cant get geo transform from [" << dataset.GetDescription() << "]";
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

  PixelCoordinates upperLeftPixPosition() const
  {
    return {{}, upperLeftPixX(), upperLeftPixY() };
  }

  bool copyToDataset(GDALDataset &dataset) const
  {
    double locGeoTransform[6];
    std::copy_n(std::begin(m_gdalGeotransform), 6, std::begin(locGeoTransform));
    return dataset.SetGeoTransform(locGeoTransform) == CE_None;
  }

  PixelCoordinates pixPosition(double dx, double dy) const
  {
    const double Xp = upperLeftPixX() + dx * pixelWidth() + dy * m_gdalGeotransform[2];
    const double Yp = upperLeftPixY() + dx * m_gdalGeotransform[4] + dy * pixelHeight();

    return {{dx, dy}, Xp, Yp };
//    return {
//      pixel * pixelWidth() + upperLeftPixX(),
//      line * pixelHeight() + upperLeftPixY()
//    };
  }

  GeoTransform recalcOrigo(const PixelImagePosition &pixel, const PixelCoordinates &pixelCoord) const
  { 
    const double origoX = -pixel.dx * pixelWidth() + pixelCoord.Xp;
    const double origoY = -pixel.dy * pixelHeight() + pixelCoord.Yp;

    double locGeoTransform[6];
    std::copy_n(std::begin(m_gdalGeotransform), 6, std::begin(locGeoTransform));
    locGeoTransform[0] = origoX;
    locGeoTransform[3] = origoY;

    GeoTransform ret(locGeoTransform);
    return ret;
  }

private:
  template< std::size_t N >
  GeoTransform( double (&array)[N] )
  {
    static_assert(N == 6, "Only array of 6");
    std::copy_n(std::begin(array), 6, std::begin(m_gdalGeotransform));
  }

  double m_gdalGeotransform[6];

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
  LOG_DBG << source << "SRC geo transform:";
  srcGeotransform.prettyPrint();
  const auto pixCoord = srcGeotransform.pixPosition(299, 181);
  
  if(!srcGeotransform.recalcOrigo({226, 371}, pixCoord).copyToDataset(*destDataset))
  {
    LOG_ERR << "Copy to dest failed";
    return 1;
  }

  GeoTransform destGeotransform(*destDataset);  
  LOG_DBG << dest << "DEST geo transform:";
  destGeotransform.prettyPrint();
  LOG_DBG << "";

  //srcGeotransform.upperLeftPixPosition().prettyPrint();

  /*
  

  GDALClose(srcDataset);
  GDALClose(destDataset);
  */

  return 0;
}


int main(int argc, char *argv[])
{
  return MainApplication::run<GdalMess>(argc, argv);
}
