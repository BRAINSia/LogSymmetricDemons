#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"

#include <iostream>

int main( int argc, char *argv[] )
{
  if ( argc < 3 )
  {
    std::cout << " Usage: " << std::endl;
    std::cout << argv[ 0 ] << std::endl;
    std::cout << " < inputImageFile > " << std::endl;
    std::cout << " < outputImageFile > " << std::endl;
    return EXIT_FAILURE;
  }

  const unsigned int Dimension = 3;
  typedef itk::Vector< short, Dimension > VectorPixelType;
  typedef itk::Image< VectorPixelType, Dimension > VelocityFieldType;
 
  typedef itk::ImageFileReader< VelocityFieldType > ReaderType;
  ReaderType::Pointer reader = ReaderType::New();
  reader->SetFileName( argv[ 1 ] );
  reader->Update();

  typedef itk::ImageFileWriter< VelocityFieldType > WriterType;
  WriterType::Pointer writer = WriterType::New();
  writer->SetFileName( argv[ 2 ] );
  writer->SetInput( reader->GetOutput() );
  writer->Update();

  return EXIT_SUCCESS;
}

