#ifndef PDY_GDAL_SCAPS_HEADER_H_
#define PDY_GDAL_SCAPS_HEADER_H_

#include <gdal/gdal.h>
#include <gdal/gdal_priv.h>
#include <gdal/cpl_conv.h>

#include <gdal/gdalwarper.h>

inline void datasetInfo(GDALDataset *dataset)
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


inline void fetchBand(GDALDataset *dataset)
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


#endif
