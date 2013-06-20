##############################################################################
#
# Library:   TubeTK
#
# Copyright 2010 Kitware Inc. 28 Corporate Drive,
# Clifton Park, NY, 12065, USA.
#
# All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#       http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
##############################################################################

set( proj Applications )

set( TubeTK_${proj}_MODULES
  AtlasBuilderUsingIntensity
  ComputeBinaryImageSimilarityMetrics
  ComputeImageSimilarityMetrics
  ComputeImageStatisticsUsingMask
  ComputeTubeGraphProbability
  ComputeTubeProbability
  #ConvertDICOMSeriesToImages # TODO: Has not been updated for GDCM 2.0.
  ConvertToMetaImage
  CropImage
  DeblendTomosynthesisSlicesUsingPrior
  EnhanceCoherenceAndEdgesUsingDiffusion
  EnhanceCoherenceUsingDiffusion
  EnhanceContrastUsingPrior
  EnhanceEdgesUsingDiffusion
  EnhanceTubesUsingDiffusion
  EnhanceUsingDiscriminantAnalysis
  EnhanceUsingNJetDiscriminantAnalysis
  ImageMath
  MergeAdjacentImages
  MergeTubeGraphs
  ResampleImage
  RegisterImageToTubesUsingRigidTransform
  RegisterUsingImageCenters
  SampleCLIApplication
  SegmentBinaryImageSkeleton
  SegmentConnectedComponents
  SegmentConnectedComponentsUsingParzenPDFs
  #SegmentTubeSeeds
  #SegmentTubes
  SegmentUsingOtsuThreshold
  SimulateAcquisitionArtifactsUsingPrior
  TubeGraphToImage
  TubesToDensityImage
  TubesToImage
  TubeToTubeGraph
  TubeTransform
  )

set( TubeTK_${proj}_Boost_MODULES )
if( TubeTK_USE_Boost )
  set( TubeTK_${proj}_Boost_MODULES
    ComputeImageQuantiles
    TransferLabelsToRegions
    TubeGraphKernel
    )
endif( TubeTK_USE_Boost )

set( TubeTK_${proj}_VTK_MODULES )
if( TubeTK_USE_VTK )
  set( TubeTK_${proj}_VTK_MODULES
    RegisterUsingSlidingGeometries
    )
endif( TubeTK_USE_VTK )

list( APPEND TubeTK_${proj}_MODULES
  ${TubeTK_${proj}_Boost_MODULES}
  ${TubeTK_${proj}_VTK_MODULES}
  )

if( NOT TubeTK_SOURCE_DIR )
  find_package( TubeTK REQUIRED )
  include( ${TubeTK_USE_FILE} )
endif( NOT TubeTK_SOURCE_DIR )

include ( ${TubeTK_CMAKE_EXTENSIONS_DIR}/TubeTKMacroAddModules.cmake )
TubeTKAddModules( MODULES ${TubeTK_${proj}_MODULES} )
