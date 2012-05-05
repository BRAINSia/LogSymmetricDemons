#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#include <iostream>

#include "itkVector.h"
#include "itkIndex.h"
#include "itkImage.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "itkExponentialDeformationFieldImageFilter2.h"
#include "itkDisplacementToVelocityFieldLogFilter.h"
#include "vnl/vnl_math.h"
#include <vnl/vnl_random.h>
#include "itkCommand.h"



// The following three classes are used to support callbacks
// on the filter in the pipeline that follows later
class ShowProgressObject
{
public:
  ShowProgressObject(itk::ProcessObject* o)
  {m_Process = o;}
  void ShowProgress()
  {std::cout << "Progress " << m_Process->GetProgress() << std::endl;}
  itk::ProcessObject::Pointer m_Process;
};



int main(int, char* [] )
{
  const unsigned int ImageDimension = 2;

  typedef itk::Vector<double,ImageDimension>    VectorType;
  typedef itk::Image<VectorType,ImageDimension> FieldType;

  typedef FieldType::PixelType  PixelType;
  typedef FieldType::IndexType  IndexType;

  typedef itk::ImageRegionIteratorWithIndex<FieldType> FieldIterator;
  
  bool testPassed = true;

  // Random number generator
  vnl_random rng;
  const double power = 0.5;

  //=============================================================

  std::cout << "Create the velocity field." << std::endl;
  
  FieldType::RegionType region;
  FieldType::SizeType size = {{64, 64}};
  region.SetSize( size );
  
  FieldType::Pointer field = FieldType::New();
  // Set LargestPossibleRegion, BufferedRegion, and RequestedRegion simultaneously.
  field->SetRegions( region );
  field->Allocate();

  // Fill the field with random values
  FieldIterator iter( field, field->GetRequestedRegion() );
  
  for ( iter.GoToBegin(); !iter.IsAtEnd(); ++iter )
    {
    PixelType & value = iter.Value();
    for ( unsigned int  i=0; i<ImageDimension; ++i )
      {
       value[i] = power * rng.normal();
      }
    }


  //=============================================================

  std::cout << "Exponentiate the velocity field." << std::endl;

  typedef itk::ExponentialDeformationFieldImageFilter<FieldType, FieldType> ExpFilterType;
  ExpFilterType::Pointer exper = ExpFilterType::New();
  exper->SetInput ( field );

  exper->Update();

  //=============================================================
  
  std::cout << "Log the deformation field with progress.";
  std::cout << std::endl;

  typedef itk::DisplacementToVelocityFieldLogFilter<FieldType,FieldType> LogFilterType;
  LogFilterType::Pointer loger = LogFilterType::New();

  loger->SetInput( exper->GetOutput() );
  loger->SetNumberOfIterations ( 5 );
  loger->SmoothVelocityFieldOff();
  loger->SetNumberOfExponentialIntegrationSteps (500);
  loger->SetNumberOfBCHApproximationTerms (3);


  ShowProgressObject progressWatch(loger);
  itk::SimpleMemberCommand<ShowProgressObject>::Pointer command;
  command = itk::SimpleMemberCommand<ShowProgressObject>::New();
  command->SetCallbackFunction(&progressWatch,
                               &ShowProgressObject::ShowProgress);
  loger->AddObserver(itk::ProgressEvent(), command);

  loger->Print( std::cout );


  loger->Update();

  
  //=============================================================

  std::cout << "Checking the output against expected." << std::endl;

  FieldIterator iter1 (field,              loger->GetOutput()->GetRequestedRegion());
  FieldIterator iter2 (loger->GetOutput(), loger->GetOutput()->GetRequestedRegion());

  double squareDiff = 0.0;
  while (!iter1.IsAtEnd() )
  {
    const FieldType::PixelType & init       = iter1.Get();
    const FieldType::PixelType & calculated = iter2.Get();

    std::cout << init << " " << calculated << std::endl;
    //    getchar();
    
    squareDiff += (init - calculated).GetSquaredNorm();
    
    ++iter1;
    ++iter2;
  }

  squareDiff /= loger->GetOutput()->GetRequestedRegion().GetNumberOfPixels();

  
  std::cout << "RMSE: " << squareDiff << std::endl;

  if ( squareDiff > 0.006 ) testPassed = false;
  
  if ( !testPassed )
    {
    std::cout << "Test failed." << std::endl;
    return EXIT_FAILURE;
    }


  std::cout << "Test passed." << std::endl;
  return EXIT_SUCCESS;

}
