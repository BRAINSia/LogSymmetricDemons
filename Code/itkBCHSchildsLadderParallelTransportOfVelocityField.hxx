#ifndef __itkBCHSchildsLadderParallelTransportOfVelocityField_txx
#define __itkBCHSchildsLadderParallelTransportOfVelocityField_txx
#include "itkBCHSchildsLadderParallelTransportOfVelocityField.h"

#include <itkProgressAccumulator.h>

namespace itk
{

/**
 * Default constructor.
 */
template <class TInputImage, class TOutputImage>
BCHSchildsLadderParallelTransportOfVelocityField<TInputImage, TOutputImage>
::BCHSchildsLadderParallelTransportOfVelocityField()
{
  // Setup the number of required inputs
  this->SetNumberOfRequiredInputs( 2 );

  // By default we shouldn't be inplace
  this->InPlaceOff();

  // Set number of apprximation terms to default value
  m_NumberOfApproximationOrder = 5;

  // Declare sub filters
  m_Adder = AdderType::New();
  m_LieBracketFilterFirstOrder = LieBracketFilterType::New();
  m_LieBracketFilterSecondOrder = LieBracketFilterType::New();
  m_MultiplierByHalfSVF = MultiplierType::New();
  m_MultiplierByHalfBracket = MultiplierType::New();

  // Multipliers can always be inplace here
  m_MultiplierByHalfSVF->InPlaceOn();
  m_MultiplierByHalfBracket->InPlaceOn();

  m_MultiplierByHalfSVF->SetConstant( 0.5 );
  m_MultiplierByHalfBracket->SetConstant( 0.5 );
}
  
/**
 * Default destructor.
 */
template <class TInputImage, class TOutputImage>
BCHSchildsLadderParallelTransportOfVelocityField<TInputImage, TOutputImage>
::~BCHSchildsLadderParallelTransportOfVelocityField()
{}


/**
 * Standard PrintSelf method.
 */
template <class TInputImage, class TOutputImage>
void
BCHSchildsLadderParallelTransportOfVelocityField<TInputImage, TOutputImage>
::PrintSelf(std::ostream& os, Indent indent) const
{
  Superclass::PrintSelf(os, indent);

  os << indent << "Adder: " << m_Adder << std::endl;
  os << indent << "LieBracketFilterFirstOrder: " << m_LieBracketFilterFirstOrder << std::endl;
  os << indent << "LieBracketFilterSecondOrder: " << m_LieBracketFilterSecondOrder << std::endl;
  os << indent << "MultiplierByHalf: " << m_MultiplierByHalfBracket << std::endl;
  os << indent << "NumberOfApproximationOrder: " << m_NumberOfApproximationOrder << std::endl;
}

/**
 * GenerateData()
 */
template <class TInputImage, class TOutputImage>
void
BCHSchildsLadderParallelTransportOfVelocityField<TInputImage, TOutputImage>
::GenerateData()
{
  InputFieldConstPointer leftField = this->GetInput(0);
  InputFieldConstPointer rightField = this->GetInput(1);

  // Create a progress accumulator for tracking the progress of minipipeline
  ProgressAccumulator::Pointer progress = ProgressAccumulator::New();

  progress->SetMiniPipelineFilter(this);

  switch( m_NumberOfApproximationOrder )
    {
    case 5:
      {
      progress->RegisterInternalFilter(m_LieBracketFilterFirstOrder, 0.5);
      progress->RegisterInternalFilter(m_MultiplierByHalfBracket, 0.2);
      progress->RegisterInternalFilter(m_Adder, 0.3);

      // u = leftField
      // v = rightField
      m_MultiplierByHalfSVF->SetInput( rightField ); 
      m_LieBracketFilterFirstOrder->SetInput( 0, m_MultiplierByHalfSVF->GetOutput() );
      m_LieBracketFilterFirstOrder->SetInput( 1, leftField );

      m_LieBracketFilterSecondOrder->SetInput( 0, m_MultiplierByHalfSVF->GetOutput() );
      m_LieBracketFilterSecondOrder->SetInput( 1, m_LieBracketFilterFirstOrder->GetOutput() );
      m_MultiplierByHalfBracket->SetInput( m_LieBracketFilterSecondOrder->GetOutput() );

      // u + [v, u] + 0.5 * [v, [v, u]]
      // But v = v/2 due to Schild's ladder construct
      // Therefore, BCH approx will be u + [0.5*v, u] + 0.5 * [0.5*v, [0.5*v, u]]
      m_Adder->SetInput( 0, leftField );
      m_Adder->SetInput( 1, m_LieBracketFilterFirstOrder->GetOutput() );
      m_Adder->SetInput( 2, m_MultiplierByHalfBracket->GetOutput() );
  
#if ( ITK_VERSION_MAJOR < 3 ) || ( ITK_VERSION_MAJOR == 3 && ITK_VERSION_MINOR < 13 )
      // Work-around for http://www.itk.org/Bug/view.php?id=8672
      m_Adder->InPlaceOff();
#else
      // Adder can be inplace since the 0th input is a temp field
      m_Adder->InPlaceOn();
#endif
      break;
      }
    default:
      {
      itkExceptionMacro(<< "NumberOfApproximationOrder ("
                        << m_NumberOfApproximationOrder << ") not supported");
      }
    }

  m_Adder->GraftOutput( this->GetOutput() );
  m_Adder->Update();
  this->GraftOutput( m_Adder->GetOutput() );
}

} // end namespace itk

#endif
