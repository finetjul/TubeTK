/*=========================================================================

Library:   TubeTK

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved.

Licensed under the Apache License, Version 2.0 ( the "License" );
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/
#ifndef __itkTubeRidgeSeedFilter_txx
#define __itkTubeRidgeSeedFilter_txx

#include <limits>

#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkImageRegionConstIteratorWithIndex.h>
#include <itkProgressReporter.h>
#include <itkTimeProbesCollectorBase.h>

#include "tubeMatrixMath.h"
#include "itkTubeRidgeSeedFilter.h"

namespace itk
{

namespace tube
{

template< class ImageT, class LabelmapT >
RidgeSeedFilter< ImageT, LabelmapT >
::RidgeSeedFilter( void )
{
  m_SeedFeatureGenerator = SeedFeatureGeneratorType::New();
  m_RidgeFeatureGenerator = RidgeFeatureGeneratorType::New();
  m_SeedFeatureGenerator->SetInputFeatureVectorGenerator(
    m_RidgeFeatureGenerator );

  m_PDFSegmenter = PDFSegmenterType::New();
}

template< class ImageT, class LabelmapT >
RidgeSeedFilter< ImageT, LabelmapT >
::~RidgeSeedFilter( void )
{
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetInput( typename ImageType::Pointer img )
{
  m_SeedFeatureGenerator->SetInputImage( img );
  m_RidgeFeatureGenerator->SetInputImage( img );
}

template < class ImageT, class LabelmapT >
typename ImageT::Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetInput( void )
{
  return m_SeedFeatureGenerator->GetInputImage();
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetLabelmap( typename LabelmapType::Pointer img )
{
  m_SeedFeatureGenerator->SetLabelmap( img );

  m_PDFSegmenter->SetLabelmap( img );
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::LabelmapType::Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetLabelmap( void )
{
  return m_SeedFeatureGenerator->GetLabelmap();
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::SeedFeatureGeneratorType::
Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetSeedFeatureGenerator( void )
{
  return m_SeedFeatureGenerator;
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::RidgeFeatureGeneratorType::
Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetRidgeFeatureGenerator( void )
{
  return m_RidgeFeatureGenerator;
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT>::PDFSegmenterType::Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetPDFSegmenter( void )
{
  return m_PDFSegmenter;
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetIntensityRange( float intensityMin, float intensityMax )
{
  m_RidgeFeatureGenerator->SetIntensityRange( intensityMin, intensityMax );
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetIntensityMin( float intensityMin )
{
  m_RidgeFeatureGenerator->SetIntensityMin( intensityMin );
}

template < class ImageT, class LabelmapT >
float
RidgeSeedFilter< ImageT, LabelmapT >
::GetIntensityMin( void )
{
  return m_RidgeFeatureGenerator->GetIntensityMin( );
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetIntensityMax( float intensityMax )
{
  m_RidgeFeatureGenerator->SetIntensityMax( intensityMax );
}

template < class ImageT, class LabelmapT >
float
RidgeSeedFilter< ImageT, LabelmapT >
::GetIntensityMax( void )
{
  return m_RidgeFeatureGenerator->GetIntensityMax( );
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetIntensityRangeByPercentile( float percentile )
{
  m_RidgeFeatureGenerator->SetIntensityRangeByPercentile( percentile );
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetScales( const RidgeScalesType & scales )
{
  m_RidgeFeatureGenerator->SetScales( scales );
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::RidgeScalesType
RidgeSeedFilter< ImageT, LabelmapT >
::GetScales( void )
{
  return m_RidgeFeatureGenerator->GetScales();
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::SetObjectId( ObjectIdType id )
{
  m_SeedFeatureGenerator->SetObjectId( id );
  m_PDFSegmenter->SetObjectId( id );
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::AddObjectId( ObjectIdType id )
{
  m_SeedFeatureGenerator->AddObjectId( id );
  m_PDFSegmenter->AddObjectId( id );
}

template < class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::ObjectIdType
RidgeSeedFilter< ImageT, LabelmapT >
::GetObjectId( unsigned int num ) const
{
  return m_PDFSegmenter->GetObjectId( num );
}

template < class ImageT, class LabelmapT >
unsigned int
RidgeSeedFilter< ImageT, LabelmapT >
::GetNumberOfObjectIds( void ) const
{
  return m_PDFSegmenter->GetNumberOfObjectIds();
}

template < class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::Update( void )
{
  m_SeedFeatureGenerator->GenerateBasis();
  m_PDFSegmenter->SetInputVolume( 0,
    m_SeedFeatureGenerator->GetFeatureImage( 0 ) );
  m_PDFSegmenter->SetInputVolume( 1,
    m_SeedFeatureGenerator->GetFeatureImage( 1 ) );
  m_PDFSegmenter->SetInputVolume( 2,
    m_SeedFeatureGenerator->GetFeatureImage( 2 ) );
  m_PDFSegmenter->Update();
}

template <class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::ClassifyImages( void )
{
  typename LabelmapType::Pointer tmpLabelmap =
    m_SeedFeatureGenerator->GetLabelmap();
  m_SeedFeatureGenerator->SetLabelmap( NULL );
  m_PDFSegmenter->SetInputVolume( 0,
    m_SeedFeatureGenerator->GetFeatureImage( 0 ) );
  m_PDFSegmenter->SetInputVolume( 1,
    m_SeedFeatureGenerator->GetFeatureImage( 1 ) );
  m_PDFSegmenter->SetInputVolume( 2,
    m_SeedFeatureGenerator->GetFeatureImage( 2 ) );
  m_PDFSegmenter->ClassifyImages();
  m_SeedFeatureGenerator->SetLabelmap( tmpLabelmap );
}

template <class ImageT, class LabelmapT >
typename RidgeSeedFilter< ImageT, LabelmapT >::LabelmapType::Pointer
RidgeSeedFilter< ImageT, LabelmapT >
::GetOutput( void )
{
  return m_PDFSegmenter->GetLabelmap();
}


template <class ImageT, class LabelmapT >
void
RidgeSeedFilter< ImageT, LabelmapT >
::PrintSelf( std::ostream & os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "PDFSegmenter = " << m_PDFSegmenter << std::endl;
  os << indent << "RidgeFeatureGenerator = " << m_RidgeFeatureGenerator
    << std::endl;
  os << indent << "SeedFeatureGenerator = " << m_SeedFeatureGenerator
    << std::endl;
}

} // tube namespace

} // itk namespace

#endif //RidgeSeedFilter_txx