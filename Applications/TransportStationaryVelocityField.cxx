#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVelocityFieldLieBracketFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itkBCHSchildsLadderParallelTransportOfVelocityField.h"
#include "itkExponentialDisplacementFieldImageFilter.h"
#include "itkWarpImageFilter.h"

#include <iostream>

int main( int argc, char *argv[] )
{
  if ( argc < 6 )
  {
    std::cout << " Usage: " << std::endl;
    std::cout << argv[ 0 ] << std::endl;
    std::cout << "  < SVF_Time0_to_Time1 >" << std::endl;
    std::cout << "  < SVF_Time1_to_Template > " << std::endl;
    std::cout << "  < Transported_SVF > " << std::endl;
    std::cout << "  < Trasnported_DefField > " << std::endl;
    std::cout << "  < Template_Image > " << std::endl;
    std::cout << "  < Warped_Template_Image > " << std::endl;
    return -1;
  }

  std::ostringstream oss;

  try
  {
    typedef itk::Vector< float, 2 > VectorPixelType;
    typedef itk::Image< VectorPixelType, 2 > VelocityFieldType;
    typedef itk::ImageFileReader< VelocityFieldType > VelocityFieldReaderType;

    // Read in stationary velocity fields
    // SVFTime0ToTime1 = u
    VelocityFieldReaderType::Pointer SVFTime0ToTime1 = VelocityFieldReaderType::New();
    SVFTime0ToTime1->SetFileName( argv[ 1 ] );
    SVFTime0ToTime1->Update();

    // Read in stationary velocity fields
    // SVFTime1ToTemplate = v
    VelocityFieldReaderType::Pointer SVFTime1ToTemplate = VelocityFieldReaderType::New();
    SVFTime1ToTemplate->SetFileName( argv[ 2 ] );
    SVFTime1ToTemplate->Update();
    
    // Create Lie Brackets needed for 2nd order approximation of
    // BCH( v, BCH( u, -v ) ) or u + [ v, u ] + 0.5 * [ v, [ v, u ] ]
    typedef itk::BCHSchildsLadderParallelTransportOfVelocityField< VelocityFieldType, VelocityFieldType > BCHType;
    BCHType::Pointer myBCHer = BCHType::New();
    myBCHer->SetNumberOfApproximationOrder( 5 );
    myBCHer->SetInput( 0, SVFTime0ToTime1->GetOutput() );
    myBCHer->SetInput( 1, SVFTime1ToTemplate->GetOutput() );
    myBCHer->Update();
    
    // Write output transported SVF
    typedef itk::ImageFileWriter< VelocityFieldType > VelocityFieldWriterType;
    VelocityFieldWriterType::Pointer velocityFieldWriter = VelocityFieldWriterType::New();
    velocityFieldWriter->SetFileName( argv[ 3 ] );
    velocityFieldWriter->SetInput( myBCHer->GetOutput() );
    velocityFieldWriter->Update();

    // Convert transported SVF to deformation/displacement field
    typedef itk::Image< VectorPixelType, 2 > DeformationFieldType;
    typedef itk::ExponentialDisplacementFieldImageFilter< VelocityFieldType, DeformationFieldType > FieldExponentiatorType;
    FieldExponentiatorType::Pointer convertVelToDispField = FieldExponentiatorType::New();
    convertVelToDispField->SetInput( myBCHer->GetOutput() );
    convertVelToDispField->ComputeInverseOff();
    convertVelToDispField->Update();
    DeformationFieldType::Pointer defField = DeformationFieldType::New();
    defField = convertVelToDispField->GetOutput();

    // Write output deformation/displacement field
    typedef itk::ImageFileWriter< DeformationFieldType > DeformationFieldWriterType;
    DeformationFieldWriterType::Pointer deformationfieldWriter = DeformationFieldWriterType::New();
    deformationfieldWriter->SetFileName( argv[ 4 ] );
    deformationfieldWriter->SetInput( convertVelToDispField->GetOutput() );
    deformationfieldWriter->Update();

    // Read in template image
    typedef float PixelType;
    typedef itk::Image< PixelType, 2 > ImageType;
    typedef itk::ImageFileReader< ImageType > TemplateImageReaderType;
    TemplateImageReaderType::Pointer templateImageReader = TemplateImageReaderType::New();
    templateImageReader->SetFileName( argv[ 5 ] );
    templateImageReader->Update();
    ImageType::Pointer templateImage = ImageType::New();
    templateImage = templateImageReader->GetOutput();

    // Warp template image with transported SVF
    typedef itk::WarpImageFilter< ImageType, ImageType, DeformationFieldType > WarperType;
    WarperType::Pointer warper = WarperType::New();
    warper->SetInput( templateImage );
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
    warpedTemplateImageWriter->SetFileName( argv[ 6 ] );
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


