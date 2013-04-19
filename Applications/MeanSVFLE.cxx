#include "itkImage.h"
#include "itkVector.h"
#include "itkImageFileReader.h"
#include "itkImageFileWriter.h"
#include "itkImageRegionIteratorWithIndex.h"
#include "vnl_sd_matrix_tools.h"
#include <iostream>

int main( int argc, char* argv[] )
{
  if( argc < 3 )
  {
    std::cout << " Useage: " << std::endl;
    std::cout << argv[ 0 ] << std::endl;
    std::cout << " < AveragedSVF > " << std::endl;
    std::cout << " < ListOfSVFs > " << std::endl;
    return -1;
  }

  std::ostringstream oss;

  typedef itk::Vector< float, 3 > VectorPixelType;
  typedef itk::Image< VectorPixelType, 3 > VelocityFieldType;
  typedef itk::ImageFileReader< VelocityFieldType > VelocityFieldReaderType;

  unsigned int numberOfImages = argc - 2;
  std::cout << "======================================================" << std::endl;
  std::cout << "Averaging " << numberOfImages << " SVFs..." << std::endl;

  // Create a list of pointers for all the SVFs
  std::vector< VelocityFieldType::Pointer > SVFList;

  for( unsigned int i = 2; i < argc ; i++ )
  {
    VelocityFieldReaderType::Pointer velFieldReader = VelocityFieldReaderType::New();
    velFieldReader->SetFileName( argv[ i ] );
    velFieldReader->Update();
   
    std::cout << "    Reading in: " << argv[ i ] << std::endl;

    VelocityFieldType::Pointer temp = velFieldReader->GetOutput();
    SVFList.push_back( temp );
  }

  // Use first SVF as reference image for output image
  // Iterate through the first SVF to build vectors of matrices
  VelocityFieldType::Pointer averageSVF = VelocityFieldType::New();
  averageSVF->SetRegions( SVFList[ 0 ]->GetLargestPossibleRegion() );
  averageSVF->SetDirection( SVFList[ 0 ]->GetDirection() );
  averageSVF->SetOrigin( SVFList[ 0 ]->GetOrigin() );
  averageSVF->SetSpacing( SVFList[ 0 ]->GetSpacing() );
  averageSVF->Allocate();

  typedef itk::ImageRegionIteratorWithIndex< VelocityFieldType > VelocityFieldIteratorType;
  VelocityFieldIteratorType tempItr( SVFList[ 0 ], SVFList[ 0 ]->GetLargestPossibleRegion() );

  size_t N = SVFList.size();
  std::vector< float > weights( N, 1.0/static_cast< float >( N ) );
  vnl_matrix< float > Matrices( N, 3 );
  std::vector< vnl_matrix< float > > Vector( N, Matrices );

  while( ! tempItr.IsAtEnd() )
  {
    VelocityFieldType::IndexType idx = tempItr.GetIndex();

    for( unsigned int i = 0; i < N; i++ )
    {
      VelocityFieldType::PixelType tempPixel = SVFList[ i ]->GetPixel( idx );
      float xComp = tempPixel[ 0 ];
      float yComp = tempPixel[ 1 ];
      float zComp = tempPixel[ 2 ];
      
      vnl_matrix< float > tempMatrix( 1, 3 );
      tempMatrix[ 0 ][ 0 ] = xComp;
      tempMatrix[ 0 ][ 1 ] = yComp;
      tempMatrix[ 0 ][ 2 ] = zComp;

      Vector[ i ] = tempMatrix;

    }

    vnl_matrix< float > tempVectorComps = sdtools::GetLogEuclideanBarycenter( Vector, weights );
    VelocityFieldType::PixelType vectorComps;
    vectorComps[ 0 ] = tempVectorComps[ 0][ 0 ];
    vectorComps[ 1 ] = tempVectorComps[ 0][ 1 ];
    vectorComps[ 2 ] = tempVectorComps[ 0][ 2 ];

    averageSVF->SetPixel( idx, vectorComps );
    averageSVF->Update();

    ++tempItr;
  }
//
//  typedef itk::ImageFileWriter< VelocityFieldType > ImageWriterType;
//  ImageWriterType::Pointer writer = ImageWriterType::New();
//  writer->SetFileName( argv[ 1 ] );
//  writer->SetInput( averageSVF );
//  writer->Update();

}
