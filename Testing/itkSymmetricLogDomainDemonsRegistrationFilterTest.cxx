#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>

#include "itkSymmetricLogDomainDemonsRegistrationFilter.h"

#include "itkVectorCastImageFilter.h"
#include "itkNearestNeighborInterpolateImageFunction.h"
#include "itkWarpImageFilter.h"

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
  const unsigned int ImageDimension = 2;

  typedef itk::Vector<float, ImageDimension>     VectorType;
  typedef itk::Image<VectorType, ImageDimension> FieldType;
  typedef itk::Image<float, ImageDimension>      ImageType;

  typedef FieldType::PixelType PixelType;
  typedef FieldType::IndexType IndexType;

  bool testPassed = true;

  // --------------------------------------------------------
  std::cout << "Generate input images and initial deformation field";
  std::cout << std::endl;

  ImageType::RegionType region;
  ImageType::SizeType   size = {{128, 128}};
  ImageType::IndexType  index;
  index.Fill( 0 );
  region.SetSize( size );
  region.SetIndex( index );

  ImageType::DirectionType direction;
  direction.SetIdentity();
  direction(1, 1) = -1;

  ImageType::Pointer moving = ImageType::New();
  ImageType::Pointer fixed = ImageType::New();
  FieldType::Pointer initField = FieldType::New();

  moving->SetRegions( region );
  moving->SetDirection( direction );
  moving->Allocate();

  fixed->SetRegions( region );
  fixed->SetDirection( direction );
  fixed->Allocate();

  initField->SetRegions( region );
  initField->SetDirection( direction );
  initField->Allocate();

  double               center[ImageDimension];
  double               radius = 30.0;
  ImageType::PixelType fgnd = 250.0;
  ImageType::PixelType bgnd = 15.0;

  // Fill the moving image with a circle
  center[0] = 64; center[1] = 64;
  FillWithCircle<ImageType>( moving, center, radius, fgnd, bgnd );

  // Fill the fixed image with a circle
  center[0] = 62; center[1] = 64;
  FillWithCircle<ImageType>( fixed, center, radius, fgnd, bgnd );

  // Fill initial velocity field with null vectors
  VectorType zeroVec;
  zeroVec.Fill( 0.0 );
  initField->FillBuffer( zeroVec );

#if 0
  typedef itk::VectorCastImageFilter<FieldType, FieldType> CasterType;
  CasterType::Pointer caster = CasterType::New();
  caster->SetInput( initField );
  caster->InPlaceOn();
#endif

  // -------------------------------------------------------------

  std::cout << "Run registration and warp moving" << std::endl;

  typedef itk::SymmetricLogDomainDemonsRegistrationFilter<ImageType, ImageType, FieldType> RegistrationType;
  RegistrationType::Pointer registrator = RegistrationType::New();

  registrator->SetInitialVelocityField( initField );
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
  registrator->InPlaceOn();

  typedef RegistrationType::DemonsRegistrationFunctionType FunctionType;
  FunctionType * fptr;
  fptr = dynamic_cast<FunctionType *>( registrator->GetDifferenceFunction().GetPointer() );
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
    return EXIT_FAILURE;
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
    registrator->SetInput( initField );
    registrator->SetMovingImage( NULL );
    registrator->Update();
    }
  catch( itk::ExceptionObject & err )
    {
    std::cout << err << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    std::cout << "Excpected exception properly handled by ignoring the above error." << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    }
  catch( ... )
    {
    std::cout << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    std::cout << "UNKNOWN exception properly handled by ignoring the above error." << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
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
    std::cout << err << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    std::cout << "Excpected exception properly handled by ignoring the above error." << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    }

  if( !testPassed )
    {
    std::cout << "Test failed" << std::endl;
    return EXIT_FAILURE;
    }

  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;
}
