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

// Qt includes
#include <QDebug>
#include <QDialogButtonBox>
#include <QFileDialog>

// QtImageViewer includes
#include "QtGlSliceView.h"
#include "ui_QtSlicerHelpGUI.h"

// ImageEditor includes
#include "QtImageEditor.h"
#include "QtOverlayControlsWidget.h"
#include "ui_QtOverlayControlsWidgetGUI.h"

// TubeTK includes
#include "itktubeGaussianDerivativeImageSource.h"

// ITK includes
#include <itkComplexToImaginaryImageFilter.h>
#include <itkComplexToModulusImageFilter.h>
#include <itkComplexToRealImageFilter.h>
#include <itkFFTShiftImageFilter.h>
#include <itkForwardFFTImageFilter.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkInverseFFTImageFilter.h>
#include <itkMultiplyImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkTimeProbe.h>

namespace tube
{

class QtImageEditor::Internals
{
public:
  typedef QtImageEditor::ImageType                     ImageType;
  typedef itk::Image<unsigned char, 3>                 UnsignedCharImageType;

  typedef itk::ImageFileReader<ImageType>              ReaderType;
  typedef itk::ImageFileWriter<UnsignedCharImageType>  WriterType;

  typedef itk::ForwardFFTImageFilter<ImageType>        FFTType;
  typedef FFTType::OutputImageType                     ComplexImageType;
  typedef itk::InverseFFTImageFilter<ComplexImageType, ImageType>
                                                       InverseFFTType;
  typedef itk::tube::GaussianDerivativeImageSource<ImageType>
                                                       GaussianDerivativeSourceType;
  typedef itk::FFTShiftImageFilter<ImageType, ImageType>
                                                       FFTShiftFilterType;
  typedef itk::MultiplyImageFilter<ComplexImageType, ImageType, ComplexImageType>
                                                       MultiplyFilterType;
protected:
  ImageType                  *m_CurrentImageData;
  ImageType                  *m_ImageData;
  QtOverlayControlsWidget    *m_OverlayWidget;
  FFTType::Pointer            m_FFTFilter;
  QLineEdit                  *m_SigmaLineEdit;
  MultiplyFilterType::Pointer m_MultiplyFilter;
  InverseFFTType::Pointer     m_InverseFFTFilter;
  QDialog                    *m_HelpDialog;

  GaussianDerivativeSourceType::Pointer createGaussianDerivative(
    GaussianDerivativeSourceType::VectorType order,
    ImageType::Pointer image);

  void setupFFTPipeline(ImageType::Pointer image);

  friend class QtImageEditor;
};


void QtImageEditor::Internals::setupFFTPipeline(ImageType::Pointer image)
{
  this->m_FFTFilter->SetInput( image );

  GaussianDerivativeSourceType::VectorType order;
  order[0] = 0;
  order[1] = 2;
  order[2] = 0;
  GaussianDerivativeSourceType::Pointer gaussianFilter =
    this->createGaussianDerivative( order, this->m_ImageData );

  FFTShiftFilterType::Pointer fftShiftFilter =
    FFTShiftFilterType::New();
  fftShiftFilter->SetInput( gaussianFilter->GetOutput() );

  this->m_MultiplyFilter->SetInput1( this->m_FFTFilter->GetOutput() );
  this->m_MultiplyFilter->SetInput2( fftShiftFilter->GetOutput() );

  this->m_InverseFFTFilter->SetInput( this->m_MultiplyFilter->GetOutput() );
}
QtImageEditor::Internals::GaussianDerivativeSourceType::Pointer
QtImageEditor::Internals::createGaussianDerivative(
  GaussianDerivativeSourceType::VectorType order,
  ImageType::Pointer image)
{
  GaussianDerivativeSourceType::Pointer gaussianDerivativeSource =
  GaussianDerivativeSourceType::New();
  gaussianDerivativeSource->SetNormalized( true );

  const ComplexImageType::RegionType inputRegion(
    image->GetLargestPossibleRegion() );
  const ComplexImageType::SizeType inputSize =
    inputRegion.GetSize();
  const ComplexImageType::SpacingType inputSpacing =
    image->GetSpacing();
  const ComplexImageType::PointType inputOrigin =
    image->GetOrigin();
  const ComplexImageType::DirectionType inputDirection =
    image->GetDirection();

  gaussianDerivativeSource->SetSize( inputSize );
  gaussianDerivativeSource->SetSpacing( inputSpacing );
  gaussianDerivativeSource->SetOrigin( inputOrigin );
  gaussianDerivativeSource->SetDirection( inputDirection );

  GaussianDerivativeSourceType::ArrayType sigma;
  GaussianDerivativeSourceType::PointType mean;
  const double sigmaLineEdit = this->m_SigmaLineEdit->text().toDouble();

  for( unsigned int ii = 0; ii < 3; ++ii )
    {
    const double halfLength = inputSize[ii]  / 2.0;
    sigma[ii] = sigmaLineEdit;
    mean[ii] = inputOrigin[ii] + halfLength;
    }
  mean = inputDirection * mean;
  gaussianDerivativeSource->SetSigma( sigma );
  gaussianDerivativeSource->SetMean( mean );
  gaussianDerivativeSource->SetOrdersVector(order);

  gaussianDerivativeSource->Update();

  return gaussianDerivativeSource;
}


QtImageEditor::QtImageEditor(QWidget* parent, Qt::WindowFlags fl ) :
  QDialog( parent, fl )
{
  this->m_Internals = new Internals;
  this->m_Internals->m_ImageData = 0;
  this->m_Internals->m_FFTFilter = Internals::FFTType::New();
  this->m_Internals->m_MultiplyFilter = Internals::MultiplyFilterType::New();
  this->m_Internals->m_InverseFFTFilter = Internals::InverseFFTType::New();

  this->setupUi(this);
  this->Controls->setSliceView(this->OpenGlWindow);

  QTabWidget *tabWidget = new QTabWidget(this);
  tabWidget->setMaximumHeight(300);

  tabWidget->insertTab(0, this->Controls, "Controls");

  QWidget *filterControlWidget = new QWidget(tabWidget);
  tabWidget->insertTab(1, filterControlWidget, "FFT Filter");

  QGridLayout *filterGridLayout = new QGridLayout(filterControlWidget);
  this->m_Internals->m_SigmaLineEdit = new QLineEdit();
  this->m_Internals->m_SigmaLineEdit->setMaximumWidth(80);
  this->m_Internals->m_SigmaLineEdit->setText("1");
  filterGridLayout->addWidget(this->m_Internals->m_SigmaLineEdit, 2, 1);

  QLabel *sigmaLabel = new QLabel(filterControlWidget);
  sigmaLabel->setText("Sigma:");
  filterGridLayout->addWidget(sigmaLabel, 2, 0);

  QPushButton *fftButton = new QPushButton();
  fftButton->setText("FFT");
  filterGridLayout->addWidget(fftButton, 0, 2);

  QPushButton *inverseFFTButton = new QPushButton();
  inverseFFTButton->setText("Inverse FFT");
  filterGridLayout->addWidget(inverseFFTButton, 1, 2);

  QPushButton *gaussianButton = new QPushButton();
  gaussianButton->setText("FFT-BLUR-INVERSE FFT");
  filterGridLayout->addWidget(gaussianButton, 2, 2);

  this->m_Internals->m_OverlayWidget = new QtOverlayControlsWidget(tabWidget);
  this->m_Internals->m_OverlayWidget->setSliceView(this->OpenGlWindow);
  tabWidget->insertTab(2, this->m_Internals->m_OverlayWidget, "Overlay");

  QDialogButtonBox *buttons = new QDialogButtonBox(Qt::Horizontal);
  buttons->addButton(this->ButtonOk, QDialogButtonBox::AcceptRole);
  QPushButton *loadImageButton = new QPushButton();
  loadImageButton->setText("Load");

  buttons->addButton(loadImageButton, QDialogButtonBox::ActionRole);
  buttons->addButton(this->ButtonHelp, QDialogButtonBox::HelpRole);


  this->gridLayout->addWidget(buttons, 2, 0,1,2);
  this->gridLayout->addWidget(tabWidget, 1, 0,1,2);

  QObject::connect(ButtonOk, SIGNAL(clicked()), this, SLOT(accept()));
  QObject::connect(ButtonHelp, SIGNAL(toggled(bool)), this, SLOT(showHelp(bool)));
  QObject::connect(SliceNumSlider, SIGNAL(sliderMoved(int)), OpenGlWindow,
                   SLOT(changeSlice(int)));
  QObject::connect(OpenGlWindow, SIGNAL(sliceNumChanged(int)), SliceNumSlider,
                   SLOT(setValue(int)));
  QObject::connect(SliceNumSlider, SIGNAL(sliderMoved(int)), this,
                   SLOT(setDisplaySliceNumber(int)));
  QObject::connect(OpenGlWindow, SIGNAL(sliceNumChanged(int)), this,
                   SLOT(setDisplaySliceNumber(int)));
  QObject::connect(fftButton, SIGNAL(clicked()), this, SLOT(applyFFT()));
  QObject::connect(inverseFFTButton, SIGNAL(clicked()), this, SLOT(applyInverseFFT()));
  QObject::connect(gaussianButton, SIGNAL(clicked()), this, SLOT(applyFilter()));
  QObject::connect(this->m_Internals->m_SigmaLineEdit, SIGNAL(textChanged(QString)), this,
                   SLOT(setDisplaySigma(QString)));
  QObject::connect(OpenGlWindow, SIGNAL(orientationChanged(int)), this,
                   SLOT(setMaximumSlice()));
//  QObject::connect(OpenGlWindow, SIGNAL(viewDetailsChanged(int)), this,
//                   SLOT(toggleTextEdit(int)));
  QObject::connect(loadImageButton, SIGNAL(clicked()), this, SLOT(loadImage()));
}

QtImageEditor::~QtImageEditor()
{
}


void QtImageEditor::showHelp(bool checked)
{
  if(!checked && this->m_Internals->m_HelpDialog != 0)
    {
    this->m_Internals->m_HelpDialog->reject();
    }
  else
    {
    this->OpenGlWindow->showHelp();
    this->m_Internals->m_HelpDialog = this->OpenGlWindow->helpWindow();
    if(this->m_Internals->m_HelpDialog != 0)
      {
      QObject::connect(this->m_Internals->m_HelpDialog, SIGNAL(rejected()),
                       this, SLOT(hideHelp()),Qt::UniqueConnection);
      }
    }
}


void QtImageEditor::hideHelp()
{
  this->ButtonHelp->setChecked(false);
}


void QtImageEditor::setInputImage(ImageType* newImageData)
{
  if (this->m_Internals->m_ImageData == 0)
    {
    this->m_Internals->m_ImageData = newImageData;
    }
  this->m_Internals->m_CurrentImageData = newImageData;
  this->OpenGlWindow->setInputImage(newImageData);
  this->setMaximumSlice();
  this->OpenGlWindow->changeSlice(((this->OpenGlWindow->maxSliceNum() -1)/2));
  this->setDisplaySliceNumber(static_cast<int>
                              (this->OpenGlWindow->sliceNum()));
  this->Controls->setInputImage();
  this->OpenGlWindow->update();

  this->m_Internals->setupFFTPipeline(this->m_Internals->m_ImageData);
}


void QtImageEditor::setDisplaySliceNumber(int number)
{
  QString tempchar = QString::number(number);
  this->SliceValue->setText(tempchar);
}


bool QtImageEditor::loadImage(QString filePathToLoad)
{
  Internals::ReaderType::Pointer reader = Internals::ReaderType::New();
  if( filePathToLoad.isEmpty() )
    {
    filePathToLoad = QFileDialog::getOpenFileName(
        0,"", QDir::currentPath());
    }

  if(filePathToLoad.isEmpty())
    {
    return false;
    }
  reader->SetFileName( filePathToLoad.toLatin1().data() );
  QFileInfo filePath(filePathToLoad);
  setWindowTitle(filePath.fileName());

  std::cout << "Loading image " << filePathToLoad.toStdString() << "... ";
  try
    {
    reader->Update();
    }
  catch (ExceptionObject & e)
    {
    std::cerr << "exception in file reader " << std::endl;
    std::cerr << e << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "done!" << std::endl;

  this->m_Internals->m_ImageData = 0;
  this->setInputImage( reader->GetOutput() );

  return true;
}


void QtImageEditor::loadOverlay(QString overlayImagePath)
{
  this->m_Internals->m_OverlayWidget->loadOverlay(overlayImagePath);
}


void QtImageEditor::setDisplaySigma(QString value)
{
  this->m_Internals->m_SigmaLineEdit->setText(value);
}


void QtImageEditor::blurFilter()
{
  /*
  this->m_FilterX = FilterType::New();
  this->m_FilterY = FilterType::New();
  this->m_FilterZ = FilterType::New();

  this->m_FilterX->SetDirection( 0 );   // 0 --> X direction
  this->m_FilterY->SetDirection( 1 );   // 1 --> Y direction
  this->m_FilterZ->SetDirection( 2 );   // 2 --> Z direction

  this->m_FilterX->SetOrder( FilterType::ZeroOrder );
  this->m_FilterY->SetOrder( FilterType::ZeroOrder );
  this->m_FilterZ->SetOrder( FilterType::ZeroOrder );

  this->m_FilterX->SetNormalizeAcrossScale( true );
  this->m_FilterY->SetNormalizeAcrossScale( true );
  this->m_FilterZ->SetNormalizeAcrossScale( true );

  this->m_FilterX->SetInput( this->m_Internals->m_FFTFilter->GetOutput() );
  this->m_FilterY->SetInput( this->m_Internals->m_FilterX->GetOutput() );
  this->m_FilterZ->SetInput( this->m_Internals->m_FilterY->GetOutput() );

  const double sigma = this->m_Internals->m_SigmaLineEdit->text().toDouble();

  this->m_FilterX->SetSigma( sigma );
  this->m_FilterY->SetSigma( sigma );
  this->m_FilterZ->SetSigma( sigma );

  this->m_FilterX->Update();
  this->m_FilterY->Update();
  this->m_FilterZ->Update();
  */
}


void QtImageEditor::applyFFT()
{
  if(this->m_Internals->m_ImageData == NULL)
    {
    qDebug() << "No image to transform";
    return;
    }

  qDebug() << "Start FFT";

  TimeProbe clockFFT;
  clockFFT.Start();
  this->m_Internals->m_FFTFilter->Update();
  clockFFT.Stop();

  qDebug() << "FFT total time:" << clockFFT.GetTotal();
}

void QtImageEditor::applyFilter()
{
  TimeProbe clockMultiply;
  clockMultiply.Start();
  this->m_Internals->m_MultiplyFilter->Update();
  clockMultiply.Stop();

  qDebug() << "Multiply total time:" << clockMultiply.GetTotal();
}

void QtImageEditor::applyInverseFFT()
{
  if(this->m_Internals->m_ImageData == NULL)
    {
    return;
    }

  qDebug()<<"Start IFFT";

  TimeProbe clockIFFT;
  clockIFFT.Start();
  this->m_Internals->m_InverseFFTFilter->Update();
  clockIFFT.Stop();

  qDebug() << "IFFT total time:" << clockIFFT.GetTotal();

  this->setInputImage(
    this->m_Internals->m_InverseFFTFilter->GetOutput());
  this->OpenGlWindow->update();
}


void QtImageEditor::displayFFT()
{
  /*
  //Extract the real part
  RealFilterType::Pointer realFilter = RealFilterType::New();
  realFilter->SetInput(this->m_Internals->m_FFTFilter->GetOutput());
  realFilter->Update();

  RescaleFilterType::Pointer realRescaleFilter = RescaleFilterType::New();
  realRescaleFilter->SetInput(realFilter->GetOutput());
  realRescaleFilter->SetOutputMinimum(0);
  realRescaleFilter->SetOutputMaximum(255);
  realRescaleFilter->Update();

  //Extract the imaginary part
  ImaginaryFilterType::Pointer imaginaryFilter = ImaginaryFilterType::New();
  imaginaryFilter->SetInput(this->m_Internals->m_FFTFilter->GetOutput());
  imaginaryFilter->Update();

  RescaleFilterType::Pointer imaginaryRescaleFilter = RescaleFilterType::New();
  imaginaryRescaleFilter->SetInput(imaginaryFilter->GetOutput());
  imaginaryRescaleFilter->SetOutputMinimum(0);
  imaginaryRescaleFilter->SetOutputMaximum(255);
  imaginaryRescaleFilter->Update();

  // Compute the magnitude
  ModulusFilterType::Pointer modulusFilter = ModulusFilterType::New();
  modulusFilter->SetInput(this->m_Internals->m_FFTFilter->GetOutput());
  modulusFilter->Update();

  RescaleFilterType::Pointer magnitudeRescaleFilter = RescaleFilterType::New();
  magnitudeRescaleFilter->SetInput(modulusFilter->GetOutput());
  magnitudeRescaleFilter->SetOutputMinimum(0);
  magnitudeRescaleFilter->SetOutputMaximum(255);
  magnitudeRescaleFilter->Update();

  // Write the images
  WriterType::Pointer realWriter = WriterType::New();
  realWriter->SetFileName("real.png");
  realWriter->SetInput(realRescaleFilter->GetOutput());
  realWriter->Update();

  WriterType::Pointer imaginaryWriter = WriterType::New();
  imaginaryWriter->SetFileName("imaginary.png");
  imaginaryWriter->SetInput(imaginaryRescaleFilter->GetOutput());
  imaginaryWriter->Update();

  WriterType::Pointer magnitudeWriter = WriterType::New();
  magnitudeWriter->SetFileName("magnitude.png");
  magnitudeWriter->SetInput(magnitudeRescaleFilter->GetOutput());
  magnitudeWriter->Update();
  */
}


void QtImageEditor::setMaximumSlice()
{
  this->SliceNumSlider->setMaximum(static_cast<int>
                                   (this->OpenGlWindow->maxSliceNum() -1));
  this->SliceNumSlider->setValue(static_cast<int>
                                 (this->SliceValue->text().toInt()));
}

} // End namespace tube
