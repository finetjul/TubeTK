/*=========================================================================

Library:   TubeTK

Copyright 2010 Kitware Inc. 28 Corporate Drive,
Clifton Park, NY, 12065, USA.

All rights reserved.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

=========================================================================*/

#include "itktubeTubeXIO.h"

int itktubeTubeXIOTest( int argc, char * argv[] )
{
  if( argc != 3 )
    {
    std::cerr << "Missing arguments." << std::endl;
    std::cerr << "Usage: " << std::endl;
    std::cerr << argv[0] << " input.tre output.tre" << std::endl;
    return EXIT_FAILURE;
    }

  // Declare the type for the Filter
  typedef itk::tube::TubeXIO< 3 >    IOMethodType;
  IOMethodType::Pointer ioMethod = IOMethodType::New();

  if( !ioMethod->Read( argv[1] ) )
    {
    return EXIT_FAILURE;
    }

  IOMethodType::TubeGroupType::Pointer tubeGroup = 
    ioMethod->GetTubeGroup();

  IOMethodType::Pointer ioMethod2 = IOMethodType::New();

  ioMethod2->SetTubeGroup( tubeGroup );

  if( !ioMethod2->Write( argv[2] ) )
    {
    return EXIT_FAILURE;
    }

  // All objects should be automatically destroyed at this point
  return EXIT_SUCCESS;
}
