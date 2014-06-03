/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkRegionFromReferenceImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2009-04-25 12:27:20 $
  Version:   $Revision: 1.10 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even 
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkRegionFromReferenceImageFilter_h
#define __itkRegionFromReferenceImageFilter_h

#include "itkExtractImageFilter.h"

namespace itk {

namespace tube {

/** \class RegionFromReferenceImageFilter
 * \brief Decrease the image size by cropping the image by an itk::Size at
 * both the upper and lower bounds of the largest possible region.
 *
 * RegionFromReferenceImageFilter changes the image boundary of an image by removing 
 * pixels outside the target region.  The target region is not specified in
 * advance, but calculated in BeforeThreadedGenerateData().
 *
 * This filter uses ExtractImageFilter to perform the cropping.
 * 
 * \ingroup GeometricTransforms
 */
template <class TInputImage, class TOutputImage=TInputImage>
class ITK_EXPORT RegionFromReferenceImageFilter:
    public ExtractImageFilter<TInputImage,TOutputImage>
{
public:
  /** Standard class typedefs. */
  typedef RegionFromReferenceImageFilter                               Self;
  typedef ExtractImageFilter<TInputImage,TOutputImage>  Superclass;
  typedef SmartPointer<Self>                            Pointer;
  typedef SmartPointer<const Self>                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);  

  /** Run-time type information (and related methods). */
  itkTypeMacro(RegionFromReferenceImageFilter, ExtractImageFilter);

  /** Typedef to describe the output and input image region types. */
  typedef typename Superclass::OutputImageRegionType OutputImageRegionType;
  typedef typename Superclass::InputImageRegionType  InputImageRegionType;

  /** Typedef to describe the type of pixel. */
  typedef typename Superclass::OutputImagePixelType OutputImagePixelType;
  typedef typename Superclass::InputImagePixelType  InputImagePixelType;

  /** Typedef to describe the output and input image index and size types. */
  typedef typename Superclass::OutputImageIndexType OutputImageIndexType;
  typedef typename Superclass::InputImageIndexType  InputImageIndexType;
  typedef typename Superclass::OutputImageSizeType  OutputImageSizeType;
  typedef typename Superclass::InputImageSizeType   InputImageSizeType;
  typedef InputImageSizeType                        SizeType;

  /** ImageDimension constants */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      Superclass::InputImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      Superclass::OutputImageDimension);
  itkStaticConstMacro(ImageDimension, unsigned int,
                      Superclass::OutputImageDimension);

  typedef ImageBase< itkGetStaticConstMacro( ImageDimension ) >    ReferenceImageType;

  /** Copy the output information from another Image. */
  void SetReferenceImage ( const ReferenceImageType *image );
  
  const ReferenceImageType * GetReferenceImage() const;

  /** Set the input image */
  void SetInput1(const TInputImage *input)
    {
    this->SetInput( input );
    }

  /** Set the reference image */
  void SetInput2(const ReferenceImageType *input)
    {
    this->SetReferenceImage( input );
    }


#ifdef ITK_USE_CONCEPT_CHECKING
  /** Begin concept checking */
  itkConceptMacro(InputConvertibleToOutputCheck,
    (Concept::Convertible<InputImagePixelType, OutputImagePixelType>));
  itkConceptMacro(SameDimensionCheck,
    (Concept::SameDimension<InputImageDimension, OutputImageDimension>));
  /** End concept checking */
#endif

protected:
  RegionFromReferenceImageFilter()
    {
    this->SetNumberOfRequiredInputs(2);
    }
  ~RegionFromReferenceImageFilter() {}

  void GenerateOutputInformation();
  
private:
  RegionFromReferenceImageFilter(const Self&); //purposely not implemented
  void operator=(const Self&); //purposely not implemented
};

  
} // end namespace tube

} // end namespace itk
  
#ifndef ITK_MANUAL_INSTANTIATION
#include "itktubeRegionFromReferenceImageFilter.hxx"
#endif
  
#endif