//
// create synthetic images for testing
#include <itkImage.h>
#include <itkImageRegionIteratorWithIndex.h>
#include <itkEllipseSpatialObject.h>
#include <itkBoxSpatialObject.h>
#include <itkImageFileWriter.h>
#include <itkOrImageFilter.h>
#include <itkXorImageFilter.h>

//
// Based on the important parameters, make and allocate an image.
template <class ImageType>
typename ImageType::Pointer
NewImage(typename ImageType::RegionType &region,
         typename ImageType::PointType &origin,
         typename ImageType::SpacingType &spacing)
{
  typename ImageType::Pointer rval = ImageType::New();
  rval->SetRegions(region);
  rval->SetOrigin(origin);
  rval->SetSpacing(spacing);
  rval->Allocate();
  rval->FillBuffer(0);
  return rval;
}

//
// Get the region in the image space that bounds
// a given SpatialObject
template <class SpatialObjectType,class ImageType>
void
GetRegion(typename ImageType::Pointer &image,
          typename SpatialObjectType::Pointer &spatialObject,
          typename ImageType::RegionType &region)
{

  typedef typename ImageType::PointType PointType;

  // used to keep spatial object's region in bounds
  const typename ImageType::RegionType &imageRegion
    = image->GetLargestPossibleRegion();

  typename SpatialObjectType::BoundingBoxType *bounds =
    spatialObject->GetBoundingBox();

  PointType minPt = bounds->GetMinimum();
  PointType maxPt = bounds->GetMaximum();
  typename ImageType::IndexType index;

  image->TransformPhysicalPointToIndex(minPt,index);
  // enlarge search region by one voxel at start
  for(unsigned int i = 0; i < ImageType::ImageDimension; ++i)
    {
    if(index[i] < 0)
      {
      index[i] = 0;
      }
    if(index[i] >= 1)
      {
      index[i]--;
      }
    }

  // make size one voxel outside the SpatialObject's bounds
  typename ImageType::SizeType size;
  for(unsigned i = 0; i < ImageType::ImageDimension; ++i)
    {
    size[i] = maxPt[i] - minPt[i] + 1;
    }

  region.SetIndex(index);
  region.SetSize(size);
  //
  // if region is outside image region, shrink it a voxel
  // at a time until it fits.
  typename ImageType::IndexType upperIndex = region.GetUpperIndex();
  while(!imageRegion.IsInside(upperIndex))
    {
    for(unsigned i = 0; i< ImageType::ImageDimension; ++i)
      {
      --size[i];
      }
    region.SetSize(size);
    upperIndex = region.GetUpperIndex();
    }
}

int main(int argc, char *argv[])
{
  if(argc != 10)
    {
    std::cerr << "MakeTestImages: Usage MakeTestImages "
              << "<circleImage> <boxImage> <rotImage> <starImage> <xorStarImage> <radiusDouble1> <radiusDouble2> <radiusDouble3> <semCirImage>"
              << std::endl;
    exit(1);
    }

  typedef itk::Image<unsigned char, 3>                 ImageType;
  typedef itk::ImageFileWriter<ImageType>              WriterType;
  typedef ImageType::IndexType                         IndexType;
  typedef ImageType::RegionType                        RegionType;
  typedef ImageType::SizeType                          SizeType;
  typedef ImageType::SpacingType                       SpacingType;
  typedef ImageType::PointType                         PointType;
  typedef itk::ImageRegionIteratorWithIndex<ImageType> ImageIteratorType;
  typedef itk::EllipseSpatialObject<3>                 EllipseType;
  typedef EllipseType::BoundingBoxType                 BoundingBoxType;
  typedef itk::BoxSpatialObject<3>                     BoxType;

  const double circleRadius1 = atof( argv[6] );
  const double circleRadius2 = atof( argv[7] );
  const double circleRadius3 = atof( argv[8] );

  // set up region for target images
  RegionType  region;
  SpacingType spacing;
  SizeType    size = {{256,256,256}};
  PointType   origin;

  spacing[2] = spacing[1] = spacing[0] = 1.0;
  origin[2] = origin[1] = origin[0] = -128.0;

  IndexType index = {{0,0,0}};
  region.SetSize(size);
  region.SetIndex(index);

  // make circle image
  ImageType::Pointer circleImage = NewImage<ImageType>(region,origin,spacing);

  EllipseType::Pointer ellipse = EllipseType::New();
  EllipseType::ArrayType radiusArray;
  radiusArray[0] = circleRadius1;
  radiusArray[1] = circleRadius2;
  radiusArray[2] = circleRadius3;
  ellipse->SetRadius(radiusArray);
  ellipse->ComputeObjectToWorldTransform();
  ellipse->ComputeLocalBoundingBox();

  RegionType ellipseRegion;
  GetRegion<EllipseType,ImageType>(circleImage,ellipse,ellipseRegion);

  // set voxels inside the spatial object
  ImageIteratorType it(circleImage,ellipseRegion);
  for(it.GoToBegin(); !it.IsAtEnd(); ++it)
    {
    const IndexType &curIndex = it.GetIndex();
    PointType curPoint;
    circleImage->TransformIndexToPhysicalPoint(curIndex,curPoint);
    if(ellipse->IsInside(curPoint))
      {
      it.Set(127);
      }
    }
  // write image
  WriterType::Pointer writer = WriterType::New();
  writer->SetInput(circleImage);
  writer->SetFileName(argv[1]);
  writer->Write();

  // make semicircle image
  ImageType::Pointer semiCircleImage = NewImage<ImageType>(region,origin,spacing);

  EllipseType::Pointer semiCircle = EllipseType::New();
  EllipseType::ArrayType circleArray;
  circleArray[0] = circleRadius1;
  circleArray[1] = circleRadius1;
  circleArray[2] = circleRadius1;
  semiCircle->SetRadius(circleArray);
  semiCircle->ComputeObjectToWorldTransform();
  semiCircle->ComputeLocalBoundingBox();

  RegionType semiCircleRegion;
  GetRegion<EllipseType,ImageType>(semiCircleImage,semiCircle,semiCircleRegion);

  // set voxels inside the spatial object
  ImageIteratorType semiIt(semiCircleImage,semiCircleRegion);
  for(semiIt.GoToBegin(); !semiIt.IsAtEnd(); ++semiIt)
    {
    const IndexType &curIndex = semiIt.GetIndex();
    PointType curPoint;
    semiCircleImage->TransformIndexToPhysicalPoint(curIndex,curPoint);
    if(semiCircle->IsInside(curPoint))
      {
      if(curIndex[2] >=128)
        {
          semiIt.Set(255);
        }
      }
    }
  // write image
  writer = WriterType::New();
  writer->SetInput(semiCircleImage);
  writer->SetFileName(argv[9]);
  writer->Write();
  
  // make a box image
  ImageType::Pointer boxImage = NewImage<ImageType>(region,origin,spacing);

  BoxType::Pointer box = BoxType::New();
  BoxType::SizeType boxSize;
  boxSize[2] = boxSize[1] = boxSize[0] = 2.0 * circleRadius1;
  box->SetSize(boxSize);

  // normally box starts with the corner at 0,0, so
  // translate it so the center of the box is at 0,0
  BoxType::TransformType::OutputVectorType boxOrigin;
  boxOrigin[2] = boxOrigin[1] = boxOrigin[0] = -circleRadius1;
  box->GetObjectToParentTransform()->SetOffset(boxOrigin);
  box->ComputeObjectToWorldTransform();
  box->ComputeLocalBoundingBox();

  RegionType boxRegion;
  GetRegion<BoxType,ImageType>(boxImage,box,boxRegion);

  ImageIteratorType boxIt(boxImage,boxRegion);

  for(boxIt.GoToBegin(); !boxIt.IsAtEnd(); ++boxIt)
    {
    const IndexType &curIndex = boxIt.GetIndex();
    PointType curPoint;
    boxImage->TransformIndexToPhysicalPoint(curIndex,curPoint);
    if(box->IsInside(curPoint))
      {
      boxIt.Set(255);
      }
    }
  writer = WriterType::New();
  writer->SetInput(boxImage);
  writer->SetFileName(argv[2]);
  writer->Write();

  //
  // make an image of an ellipse rotated 45 degrees
  ImageType::Pointer ellipseImage45 = NewImage<ImageType>(region,origin,spacing);
  // re-use the ellipse spatial object, adding a 45 deg rotation to
  // the translation. It will be rotated around its own origin, which
  // will correspond to 0,0 in world coordinates, or the center of the image.
  EllipseType::Pointer ellipse45 = EllipseType::New();
  ellipse45->SetRadius(radiusArray);
  
  EllipseType::TransformType::OutputVectorType ellipse45Origin;
  ellipse45Origin[2] = ellipse45Origin[1] = ellipse45Origin[0] = -128;
  ellipse45->GetObjectToParentTransform()->Rotate3D(ellipse45Origin, vnl_math::pi/3.0);
  ellipse45->ComputeObjectToWorldTransform();
  ellipse45->ComputeBoundingBox();

  GetRegion<EllipseType,ImageType>(ellipseImage45,ellipse45,region);

  ImageIteratorType ellipse45It2(ellipseImage45,region);
  //
  // set voxels inside ellipse45
  for(ellipse45It2.GoToBegin(); !ellipse45It2.IsAtEnd(); ++ellipse45It2)
    {
    const IndexType &curIndex = ellipse45It2.GetIndex();
    PointType curPoint;
    ellipseImage45->TransformIndexToPhysicalPoint(curIndex,curPoint);
    if(ellipse45->IsInside(curPoint))
      {
      ellipse45It2.Set(127);
      }
    }
  writer = WriterType::New();
  writer->SetInput(ellipseImage45);
  writer->SetFileName(argv[3]);
  writer->Write();

  // create 6 point star image with OR
  typedef itk::OrImageFilter<ImageType,ImageType,ImageType> OrFilterType;
  OrFilterType::Pointer orFilter = OrFilterType::New();

  orFilter->SetInput1(boxImage);
  orFilter->SetInput2(ellipseImage45);
  orFilter->Update();

  writer = WriterType::New();
  writer->SetInput(orFilter->GetOutput());
  writer->SetFileName(argv[4]);
  writer->Write();

  // xor with the circle to show how they overlap
  typedef itk::XorImageFilter<ImageType,ImageType,ImageType> XorFilterType;
  XorFilterType::Pointer xorFilter = XorFilterType::New();

  xorFilter->SetInput1(orFilter->GetOutput());
  xorFilter->SetInput2(circleImage);
  xorFilter->Update();

  writer = WriterType::New();
  writer->SetInput(xorFilter->GetOutput());
  writer->SetFileName(argv[5]);
  writer->Write();

  return EXIT_SUCCESS;
}
