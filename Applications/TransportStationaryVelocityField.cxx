#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkVelocityFieldLieBracketFilter.h"
#include "itkMultiplyImageFilter.h"
#include "itkAddImageFilter.h"
#include "itkImageToImageFilter.h"
#include "itkBCHSchildsLadderParallelTransportOfVelocityField.h"
#include "itkWarpImageFilter.h"

#include <iostream>

int main( int argc, char *argv[] )
{
  if ( argc < 4 )
  {
    std::cout << " Usage: " << std::endl;
    std::cout << argv[ 0 ] << std::endl;
    std::cout << "  < SVF_Time0_to_Time1 >" << std::endl;
    std::cout << "  < SVF_Time1_to_Template > " << std::endl;
    std::cout << "  < Transported_SVF > " << std::endl;
    std::cout << "  < Template_Image > " << std::endl;
    return -1;
  }

  std::ostringstream oss;

  try
  {
    typedef itk::Vector< float, 3 > VectorPixelType;
    typedef itk::Image< VectorPixelType, 3 > VelocityFieldType;
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
    
    // Write out transported SVF
    typedef itk::ImageFileWriter< VelocityFieldType > WriterType;
    WriterType::Pointer writer = WriterType::New();
    writer->SetFileName( argv[ 3 ] );
    writer->SetInput( myBCHer->GetOutput() );
    writer->Update();

    // Read in template image
    typedef float PixelType;
    typedef itk::Image< PixelType, 3 > ImageType;
    typedef it::ImageFileReader< ImageType > TemplateImageReaderType;
    TemplateImageReaderType::Pointer templateImageReader = TemplateImageReaderType::New();
    templateImageReader->SetFileName( argv[ 4 ] );
    templateImageReader->Update();

    

  }
  catch( itk::ExceptionObject & err )
  {
    oss << "Exception Object caught: " << std::endl;
    oss << err << std::endl;
    throw;
  }
}


