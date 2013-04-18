#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkAddImageFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkImageFileWriter.h"
#include <iostream>

int main( int argc, char* argv[] )
{
  if( argc < 3 )
  {
    std::cout << " Usaage: " << std::endl;
    std::cout << argv[ 0 ] << std::endl;
    std::cout << " < AveragedSVF > " << std::endl;
    std::cout << " < ListOfSVFs > " << std::endl;
    return -1;
  }

  std::ostringstream oss;

  typedef itk::Vector< float, 3 > VectorPixelType;
  typedef itk::Image< VectorPixelType, 3 > VelocityFieldType;
  typedef itk::ImageFileReader< VelocityFieldType > VelocityFieldReaderType;
  typedef itk::Image< float, 3 > ImageType;
  typedef itk::AddImageFilter< VelocityFieldType, VelocityFieldType, VelocityFieldType > AddImageFilterType;
  typedef itk::MultiplyImageFilter< VelocityFieldType, ImageType, VelocityFieldType > MultiplyImageFilterType;
  typedef itk::ImageDuplicator< VelocityFieldType > ImageDuplicatorType;

  unsigned int numberOfImages = argc - 2;
  std::cout << "======================================================" << std::endl;
  std::cout << "Averaging " << numberOfImages << " SVFs..." << std::endl;

  VelocityFieldReaderType::Pointer velFieldReader = VelocityFieldReaderType::New();
  AddImageFilterType::Pointer adder = AddImageFilterType::New();
  ImageDuplicatorType::Pointer dup = ImageDuplicatorType::New();

  velFieldReader->SetFileName( argv[ 2 ] );
  velFieldReader->Update();
  std::cout << " " << std::endl;
  std::cout << "Reading in first SVF: " << velFieldReader->GetFileName() << std::endl;

  VelocityFieldType::IndexType idx;
  idx[ 0 ] = 253;
  idx[ 1 ] = 221;
  idx[ 2 ] = 190;

  std::cout << "    First SVF pixel values at [253, 221, 190]: " << velFieldReader->GetOutput()->GetPixel( idx ) << std::endl;
  dup->SetInputImage( velFieldReader->GetOutput() );
  dup->Update();

  VelocityFieldType::Pointer summer = dup->GetOutput();
  std::cout << "    Dup pixel values at [253, 221, 190]: " << summer->GetPixel( idx ) << std::endl;

  for( unsigned int i = 3; i < argc ; ++i )
  {
    velFieldReader->SetFileName( argv[ i ] );
    velFieldReader->Update();
    std::cout << " " << std::endl;
    std::cout << "Reading in next SVF: " << velFieldReader->GetFileName() << std::endl;
    std::cout << "    Next SVF pixel values at [253, 221, 190]: " << velFieldReader->GetOutput()->GetPixel( idx ) << std::endl;

    adder->SetInput1( summer );
    adder->SetInput2( velFieldReader->GetOutput() );
    adder->Update();
    std::cout << "    Adder SVF pixel values at [253, 221, 190]: " << adder->GetOutput()->GetPixel( idx ) << std::endl;

    dup->SetInputImage( adder->GetOutput() );
    dup->Update();
    summer = dup->GetOutput();
  }

  std::cout << " " << std::endl;
  std::cout << "Image pixel values at [253, 221, 190] after summing: " << summer->GetPixel( idx ) << std::endl;
  
  MultiplyImageFilterType::Pointer multiplyByThis = MultiplyImageFilterType::New();
  float div = 1.0 / (float)numberOfImages;
  std::cout << "Will divide by " << numberOfImages << " or multiply by " << div << std::endl;

  multiplyByThis->SetInput( summer );
  multiplyByThis->SetConstant2( div );
  multiplyByThis->Update();

  VelocityFieldType::Pointer halfed = multiplyByThis->GetOutput();

  std::cout << " " << std::endl;
  std::cout << "TestMultiply: " << summer->GetPixel( idx ) << " x " << div << " = " << summer->GetPixel( idx ) * div << std::endl;
  std::cout << "Average image pixel values at [253, 221, 190] after dividing: " << halfed->GetPixel( idx ) << std::endl;
  std::cout << " " << std::endl;

  typedef itk::ImageFileWriter< VelocityFieldType > ImageWriterType;
  ImageWriterType::Pointer writer = ImageWriterType::New();
  writer->SetFileName( argv[ 1 ] );
  writer->SetInput( multiplyByThis->GetOutput() );
  writer->Update();
}

