#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "itkSignedMaurerDistanceMapImageFilter.h"
#include "itkImage.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "itkImageDuplicator.h"
#include "itkImageRegionIterator.h"


int main( int argc, char * argv[] )
{
  if( argc < 3 )
    {
    std::cerr << "Usage: " << argv[ 0 ];
    std::cerr << " inputImageFile outputDistanceMapImageFile zeroedDistanceMap";
    std::cerr << std::endl;  
    return EXIT_FAILURE;
    }
  
  typedef unsigned int InputPixelType;
  typedef float OutputPixelType;
  typedef itk::Image< InputPixelType, 2 > InputImageType;
  typedef itk::Image< OutputPixelType, 2 > OutputImageType;
  
  typedef itk::ImageFileReader< InputImageType  > ReaderType;
  ReaderType::Pointer binaryImageReader = ReaderType::New();
  binaryImageReader->SetFileName( argv[ 1 ] );
  binaryImageReader->Update();

  typedef itk::SignedMaurerDistanceMapImageFilter< InputImageType, OutputImageType >  FilterType;
  FilterType::Pointer filter = FilterType::New();
  filter->SetInput( binaryImageReader->GetOutput() );
  filter->Update();

  typedef itk::ImageFileWriter< OutputImageType > WriterType;
  WriterType::Pointer distMapWriter = WriterType::New();
  distMapWriter->SetFileName( argv[ 2 ] );
  distMapWriter->SetInput( filter->GetOutput() );
  distMapWriter->Update();

  typedef itk::ImageDuplicator< OutputImageType > DuplicatorType;
  DuplicatorType::Pointer dupDistMap = DuplicatorType::New();
  dupDistMap->SetInputImage( filter->GetOutput() );
  dupDistMap->Update();

  typedef itk::ImageRegionIterator< InputImageType > ImageRegionIteratorType;
  ImageRegionIteratorType BinaryImageItr( binaryImageReader->GetOutput(), binaryImageReader->GetOutput()->GetRequestedRegion() );
  for( BinaryImageItr.GoToBegin(); !BinaryImageItr.IsAtEnd(); ++BinaryImageItr )
  {
    OutputImageType::IndexType idx = BinaryImageItr.GetIndex();
    if( binaryImageReader->GetOutput()->GetPixel( idx ) == 0 )
    {
      dupDistMap->GetOutput()->SetPixel( idx, 0.0 );
      dupDistMap->Update();
    }
  }
  
  WriterType::Pointer zeroedDistMap = WriterType::New();
  zeroedDistMap->SetFileName( argv[ 3 ] );
  zeroedDistMap->SetInput( dupDistMap->GetOutput() );
  zeroedDistMap->Update();

  return EXIT_SUCCESS;
}

