#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>

#include "itkLogDomainDemonsRegistrationFilter.h"

#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkVectorCastImageFilter.h"
#include "itkWarpImageFilter.h"
#include "itkImageFileWriter.h"

/*
 * This is the prefered ABI for Writing images.
 * We know that the image is not going to change
 * so make sure that the API indicates that.
 */
template <class ImageType>
void
WriteConstImage(const typename ImageType::ConstPointer image,
                const std::string & filename)
{
  typedef itk::ImageFileWriter<ImageType> WriterType;
  typename  WriterType::Pointer writer = WriterType::New();
  writer->UseCompressionOn();
  writer->SetFileName( filename.c_str() );
  writer->SetInput(image);
  try
    {
    writer->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << "Exception Object caught: " << std::endl;
    std::cout << err << std::endl;
    throw;
    }
}

namespace
{
// The following class is used to support callbacks
// on the filter in the pipeline that follows later
template <typename TRegistration>
class ShowProgressObject
{
public:
  ShowProgressObject(TRegistration* o)
  {
    m_Process = o;
  }

  void ShowProgress()
  {
    std::cout << "Progress: " << m_Process->GetProgress() << "  ";
    std::cout << "Iter: " << m_Process->GetElapsedIterations() << "  ";
    std::cout << "Metric: "   << m_Process->GetMetric()   << "  ";
    std::cout << "RMSChange: " << m_Process->GetRMSChange() << "  ";
    std::cout << std::endl;
    if( m_Process->GetElapsedIterations() == 150 )
      {
      m_Process->StopRegistration();
      }
  }

  typename TRegistration::Pointer m_Process;
};
}

// Template function to fill in an image with a circle.
template <class TImage>
void
FillWithCircle(TImage * image,
               double * center,
               double radius,
               typename TImage::PixelType foregnd,
               typename TImage::PixelType backgnd )
{
  typedef itk::ImageRegionIteratorWithIndex<TImage> Iterator;
  Iterator it( image, image->GetBufferedRegion() );
  it.GoToBegin();

  typename TImage::IndexType index;
  double r2 = vnl_math_sqr( radius );
  for( ; !it.IsAtEnd(); ++it )
    {
    index = it.GetIndex();
    double distance = 0;
    for( unsigned int j = 0; j < TImage::ImageDimension; j++ )
      {
      distance += vnl_math_sqr( (double) index[j] - center[j]);
      }
    if( distance <= r2 )
      {
      it.Set( foregnd );
      }
    else
      {
      it.Set( backgnd );
      }
    }
}

// ----------------------------------------------

int main(int, char * [] )
{
  try
    {
    const unsigned int ImageDimension = 2;
    //HACK!
    itk::MultiThreader::SetGlobalMaximumNumberOfThreads(1);
    typedef itk::Vector<float, ImageDimension>     VectorType;
    typedef itk::Image<VectorType, ImageDimension> FieldType;
    typedef itk::Image<float, ImageDimension>      ImageType;

    typedef FieldType::PixelType PixelType;
    typedef FieldType::IndexType IndexType;

    bool testPassed = true;

    // --------------------------------------------------------
    std::cout << "Generate input images and initial deformation field";
    std::cout << std::endl;

    // Declare fixed image
    ImageType::RegionType fixed_region;
    const ImageType::SizeType   fixed_size = {{128, 128}};
    ImageType::IndexType  fixed_index;
    fixed_index.Fill( 0 );
    fixed_region.SetSize( fixed_size );
    fixed_region.SetIndex( fixed_index );

    ImageType::DirectionType fixed_direction;
    fixed_direction.SetIdentity();
    // fixed_direction(1,1)=-1;

    ImageType::SpacingType fixed_spacing;
    fixed_spacing.Fill( 1.0 );

    ImageType::PointType fixed_origin;
    fixed_origin.Fill( 0.0 );

    ImageType::Pointer fixed = ImageType::New();
    fixed->SetRegions( fixed_region );
    fixed->Allocate();
    fixed->SetDirection( fixed_direction );
    fixed->SetSpacing( fixed_spacing );
    fixed->SetOrigin( fixed_origin );

    // Fill the fixed image with a circle
    const double               radius = 30.0;
    const ImageType::PixelType fgnd = 250.0;
    const ImageType::PixelType bgnd = 15.0;

    itk::Point<double, ImageDimension> center_pt_fixed;

      {
      center_pt_fixed[0] = 62;
      center_pt_fixed[1] = 64;
      }

    itk::ContinuousIndex<double, ImageDimension> center_cind_fixed;
    fixed->TransformPhysicalPointToContinuousIndex( center_pt_fixed, center_cind_fixed);

    FillWithCircle<ImageType>( fixed, center_cind_fixed.GetDataPointer(),
      radius, fgnd, bgnd );

    WriteConstImage<ImageType>(fixed.GetPointer(),"FIXED_LDDRFT1.nii.gz");

    // Declare moving image
    ImageType::RegionType moving_region;
    ImageType::SizeType   moving_size = fixed_size;
    ImageType::IndexType  moving_index;
    moving_index.Fill( 0 );
    moving_region.SetSize( moving_size );
    moving_region.SetIndex( moving_index );

    ImageType::DirectionType moving_direction;
    // moving_direction.SetIdentity();
    moving_direction = fixed_direction;

    ImageType::SpacingType moving_spacing;
    moving_spacing.Fill( 1.0 );

    ImageType::PointType moving_origin;
    moving_origin.Fill( 0.0 );

    ImageType::Pointer moving = ImageType::New();
    moving->SetRegions( moving_region );
    moving->Allocate();
    moving->SetDirection( moving_direction );
    moving->SetSpacing( moving_spacing );
    moving->SetOrigin( moving_origin );

    // Fill the moving image with a circle
    itk::ContinuousIndex<double, ImageDimension> center_cind_moving;
    moving->TransformPhysicalPointToContinuousIndex( center_pt_fixed, center_cind_moving);
      {
      center_pt_fixed[0] = 64;
      center_pt_fixed[1] = 64;
      }

    FillWithCircle<ImageType>( moving, center_cind_moving.GetDataPointer(),
      radius, fgnd, bgnd );

    WriteConstImage<ImageType>(moving.GetPointer(), "MOVING_LDDRFT1.nii.gz");
    // -------------------------------------------------------------
    // -------------------------------------------------------------
    FieldType::Pointer initField = FieldType::New();
    initField->SetRegions( fixed_region );
    initField->Allocate();
    initField->SetDirection( fixed_direction );

    // Fill initial velocity field with null vectors
      {
      VectorType zeroVec;
      zeroVec.Fill( 0.0 );
      initField->FillBuffer( zeroVec );
      }

    typedef itk::VectorCastImageFilter<FieldType, FieldType> CasterType;
    CasterType::Pointer caster = CasterType::New();
    caster->SetInput( initField );
    caster->InPlaceOff();

    // -------------------------------------------------------------
    // -------------------------------------------------------------

    std::cout << "Run registration and warp moving" << std::endl;

    typedef itk::LogDomainDemonsRegistrationFilter<ImageType, ImageType, FieldType> RegistrationType;
    RegistrationType::Pointer registrator = RegistrationType::New();

    registrator->SetInitialVelocityField( caster->GetOutput() );
    registrator->SetMovingImage( moving );
    registrator->SetFixedImage( fixed );
    registrator->SetNumberOfIterations( 200 );
    registrator->SetStandardDeviations( 0.7 );
    registrator->SetMaximumUpdateStepLength( 2.0 );
    registrator->SetMaximumError( 0.08 );
    registrator->SetMaximumKernelWidth( 10 );
    registrator->SetIntensityDifferenceThreshold( 0.001 );
    registrator->SetNumberOfBCHApproximationTerms( 2 );

    // Turn on inplace execution
    //HACK registrator->InPlaceOn();
    registrator->InPlaceOff();

    typedef RegistrationType::DemonsRegistrationFunctionType FunctionType;
    FunctionType * fptr = dynamic_cast<FunctionType *>( registrator->GetDifferenceFunction().GetPointer() );
    fptr->Print( std::cout );

    // Exercise other member variables
    std::cout << "Max. error for Gaussian operator approximation: "
      << registrator->GetMaximumError()
      << std::endl;
    std::cout << "Max. Gaussian kernel width: "
      << registrator->GetMaximumKernelWidth()
      << std::endl;

    // Set standards deviations
    double v[ImageDimension];
    for( unsigned int j = 0; j < ImageDimension; j++ )
      {
      v[j] = registrator->GetStandardDeviations()[j];
      }
    registrator->SetStandardDeviations( v );

    // Progress tracking
    typedef ShowProgressObject<RegistrationType> ProgressType;
    ProgressType                                    progressWatch(registrator);
    itk::SimpleMemberCommand<ProgressType>::Pointer command;
    command = itk::SimpleMemberCommand<ProgressType>::New();
    command->SetCallbackFunction(&progressWatch,
      &ProgressType::ShowProgress);
    registrator->AddObserver( itk::ProgressEvent(), command);

    registrator->Update();

    // Warper for the moving image
    typedef itk::WarpImageFilter<ImageType, ImageType, FieldType> WarperType;
    WarperType::Pointer warper = WarperType::New();

    // Interpolator
    typedef WarperType::CoordRepType CoordRepType;
    typedef itk::NearestNeighborInterpolateImageFunction<ImageType, CoordRepType>
      InterpolatorType;
    InterpolatorType::Pointer interpolator = InterpolatorType::New();

    warper->SetInput( moving );
#if (ITK_VERSION_MAJOR < 4)
    warper->SetDeformationField( registrator->GetDeformationField() );
#else
    warper->SetDisplacementField( registrator->GetDeformationField() );
#endif
    warper->SetInterpolator( interpolator );
    warper->SetOutputSpacing( fixed->GetSpacing() );
    warper->SetOutputOrigin( fixed->GetOrigin() );
    warper->SetOutputDirection( fixed->GetDirection() );
    warper->SetEdgePaddingValue( bgnd );

    warper->Print( std::cout );

    warper->Update();

    // ---------------------------------------------------------

    std::cout << "Compare warped moving and fixed." << std::endl;

    itk::ImageRegionIterator<ImageType> fixedIter( fixed,
      fixed->GetBufferedRegion() );
    itk::ImageRegionIterator<ImageType> warpedIter( warper->GetOutput(),
      fixed->GetBufferedRegion() );

    ImageType::Pointer warpedOutput = warper->GetOutput();

    WriteConstImage<ImageType>( warpedOutput.GetPointer(),"WARPEDMOVING_LDDRFT1.nii.gz");

    unsigned int numPixelsDifferent = 0;
    while( !fixedIter.IsAtEnd() )
      {
      if( fixedIter.Get() != warpedIter.Get() )
        {
        numPixelsDifferent++;
        }
      ++fixedIter;
      ++warpedIter;
      }

    std::cout << "Number of pixels that differ: " << numPixelsDifferent;
    std::cout << std::endl;

    if( numPixelsDifferent > 10 )
      {
      std::cout << "Test failed - too many pixels differ." << std::endl;
      testPassed = false;
      }

    registrator->Print( std::cout );

    // -----------------------------------------------------------

    std::cout << "Test running registrator without initial deformation field.";
    std::cout << std::endl;

    try
      {
      registrator->SetInput( NULL );
      registrator->SetNumberOfIterations( 2 );
      registrator->Update();
      }
    catch( itk::ExceptionObject& err )
      {
      std::cout << "Unexpected error." << std::endl;
      std::cout << err << std::endl;
      testPassed = false;
      }

    if( !testPassed )
      {
      std::cout << "Test failed" << std::endl;
      return EXIT_FAILURE;
      }

    // --------------------------------------------------------------
    std::cout << "Test exception handling." << std::endl;
    std::cout << "Test NULL moving image. " << std::endl;
    try
      {
      registrator->SetInput( caster->GetOutput() );
      registrator->SetMovingImage( NULL );
      registrator->Update();
      }
    catch( itk::ExceptionObject & err )
      {
      std::cout << "Caught expected error." << std::endl;
      std::cout << err << std::endl;
      }
    if( !testPassed )
      {
      std::cout << "Test failed" << std::endl;
      return EXIT_FAILURE;
      }

    registrator->SetMovingImage( moving );
    registrator->ResetPipeline();
    std::cout << "Test NULL moving image interpolator. " << std::endl;

    try
      {
      fptr = dynamic_cast<FunctionType *>( registrator->GetDifferenceFunction().GetPointer() );
      fptr->SetMovingImageInterpolator( NULL );
      registrator->SetInput( initField );
      registrator->Update();
      }
    catch( itk::ExceptionObject & err )
      {
      std::cout << "Caught expected error." << std::endl;
      std::cout << err << std::endl;
      }

    if( !testPassed )
      {
      std::cout << "Test failed" << std::endl;
      return EXIT_FAILURE;
      }
    }
  catch( itk::ExceptionObject& err )
    {
    std::cout << "Failed run with exception caught." << std::endl;
    std::cout << err << std::endl;
    exit( EXIT_FAILURE );
    }

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
