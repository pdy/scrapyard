#ifndef PDY_GEOTRANSFORM_H_
#define PDY_GEOTRANSFORM_H_

#include "application/TrivialLogger.h"

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <gdal/gdalwarper.h>

#include "ScapsHeader.h"

class AffineGeoTransform
{
public:
  struct PixelImagePosition
  {
    double dx {.0};
    double dy {.0};
  };

  struct PixelCoordinates
  {
    PixelImagePosition pixelXY;
    double Xp;
    double Yp;
    void prettyPrint() const
    {
      LOG_DBG << "Pixel " << pixelXY.dx << "x" << pixelXY.dy << " maps to coord:";
      LOG_DBG << "  Xp [" << std::fixed << Xp << std::scientific << "]";
      LOG_DBG << "  Yp [" << std::fixed << Yp << std::scientific << "]";
    }
  };

  AffineGeoTransform(GDALDataset &dataset)
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
    LOG_DBG << "  upper left pixel X [" << std::fixed << upperLeftPixX() << std::scientific << "]";
    LOG_DBG << "  upper left pixel Y [" << std::fixed << upperLeftPixY() << std::scientific << "]";
  }

  PixelCoordinates upperLeftPixCoord() const
  {
    return {{.0, .0}, upperLeftPixX(), upperLeftPixY() };
  }

  bool copyToDataset(GDALDataset &dataset) const
  {
    double locAffineGeoTransform[6];
    std::copy_n(std::begin(m_gdalGeotransform), 6, std::begin(locAffineGeoTransform));
    return dataset.SetGeoTransform(locAffineGeoTransform) == CE_None;
  }

  PixelCoordinates getPixCoordinates(double dx, double dy) const
  {
    const double Xp = upperLeftPixX() + dx * pixelWidth() + dy * m_gdalGeotransform[2];
    const double Yp = upperLeftPixY() + dx * m_gdalGeotransform[4] + dy * pixelHeight();

    return {{dx, dy}, Xp, Yp };
//    return {
//      pixel * pixelWidth() + upperLeftPixX(),
//      line * pixelHeight() + upperLeftPixY()
//    };
  }

  PixelCoordinates getPixCoordinates(const PixelImagePosition &xy) const
  {
    return getPixCoordinates(xy.dx, xy.dy);
  }

  AffineGeoTransform recalcOrigo(const PixelImagePosition &pixel, const PixelCoordinates &pixelCoord) const
  { 
    const double origoX = -pixel.dx * pixelWidth() + pixelCoord.Xp;
    const double origoY = -pixel.dy * pixelHeight() + pixelCoord.Yp;

    double locAffineGeoTransform[6];
    std::copy_n(std::begin(m_gdalGeotransform), 6, std::begin(locAffineGeoTransform));
    locAffineGeoTransform[0] = origoX;
    locAffineGeoTransform[3] = origoY;

    return AffineGeoTransform(locAffineGeoTransform);
  }

private:
  template< std::size_t N >
  AffineGeoTransform( double (&array)[N] )
  {
    static_assert(N == 6, "Only array of 6");
    std::copy_n(std::begin(array), 6, std::begin(m_gdalGeotransform));
  }

  double m_gdalGeotransform[6];

};

#endif
