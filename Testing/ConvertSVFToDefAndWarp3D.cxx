#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageToImageFilter.h"
#include "itkExponentialDisplacementFieldImageFilter.h"
#include "itkWarpImageFilter.h"

#include <iostream>

int main( int argc, char *argv[] )
{
//  if ( argc < 4 )
//  {
//    std::cout << " Usage: " << std::endl;
//    std::cout << argv[ 0 ] << std::endl;
//    std::cout << "  < inputStationaryVelocityField >" << std::endl;
//    std::cout << "  < inputMovingImage > " << std::endl;
//    std::cout << "  < outputDisplacementField > " << std::endl;
//    std::cout << "  < outputWarpedImage > " << std::endl;
//    return -1;
//  }

  std::ostringstream oss;

  try
  {
    typedef itk::Vector< float, 3 > VectorPixelType;
    typedef itk::Image< VectorPixelType, 3 > VelocityFieldType;
    typedef itk::ImageFileReader< VelocityFieldType > VelocityFieldReaderType;

    // Read in stationary velocity field
    VelocityFieldReaderType::Pointer inputStatVelFieldReader = VelocityFieldReaderType::New();
    inputStatVelFieldReader->SetFileName( argv[ 1 ] );
    inputStatVelFieldReader->Update();

    // Read in moving image
    typedef itk::Image< float, 3 > ImageType;
    typedef itk::ImageFileReader< ImageType > MovingImageReaderType;
    MovingImageReaderType::Pointer movingImageReader = MovingImageReaderType::New();
    movingImageReader->SetFileName( argv[ 2 ] );
    movingImageReader->Update();
    ImageType::Pointer movingImage = ImageType::New();
    movingImage = movingImageReader->GetOutput();

    // Convert transported SVF to deformation/displacement field
    typedef itk::Image< VectorPixelType, 3 > DeformationFieldType;
    typedef itk::ExponentialDisplacementFieldImageFilter< VelocityFieldType, DeformationFieldType > FieldExponentiatorType;
    FieldExponentiatorType::Pointer convertVelToDispField = FieldExponentiatorType::New();
    convertVelToDispField->SetInput( inputStatVelFieldReader->GetOutput() );
    convertVelToDispField->ComputeInverseOff();
    convertVelToDispField->Update();
    DeformationFieldType::Pointer defField = DeformationFieldType::New();
    defField = convertVelToDispField->GetOutput();

    // Write output deformation/displacement field
    typedef itk::ImageFileWriter< DeformationFieldType > DeformationFieldWriterType;
    DeformationFieldWriterType::Pointer deformationfieldWriter = DeformationFieldWriterType::New();
    deformationfieldWriter->SetFileName( argv[ 3 ] );
    deformationfieldWriter->SetInput( convertVelToDispField->GetOutput() );
    deformationfieldWriter->Update();

    // Warp template image with transported SVF
    typedef itk::WarpImageFilter< ImageType, ImageType, DeformationFieldType > WarperType;
    WarperType::Pointer warper = WarperType::New();
    warper->SetInput( movingImage );
    warper->SetOutputSpacing( defField->GetSpacing() );
    warper->SetOutputOrigin( defField->GetOrigin() );
    warper->SetOutputDirection( defField->GetDirection() );
#if ( ITK_VERSION_MAJOR < 4 )
    warper->SetDeformationField( convertVelToDispField->GetOutput() );
#else
    warper->SetDisplacementField( convertVelToDispField->GetOutput() );
#endif

    // Writer out warped template image
    typedef itk::ImageFileWriter< ImageType > ImageWriterType;
    ImageWriterType::Pointer warpedTemplateImageWriter = ImageWriterType::New();
    warpedTemplateImageWriter->SetFileName( argv[ 4 ] );
    warpedTemplateImageWriter->SetInput( warper->GetOutput() );
    warpedTemplateImageWriter->Update();

  }
  catch( itk::ExceptionObject & err )
  {
    oss << "Exception Object caught: " << std::endl;
    oss << err << std::endl;
    throw;
  }
}



