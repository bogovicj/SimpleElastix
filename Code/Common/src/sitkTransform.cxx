#include "sitkTransform.h"
#include "sitkExceptionObject.h"

#include "itkTransformBase.h"

#include "itkIdentityTransform.h"
#include "itkTranslationTransform.h"
#include "itkScaleTransform.h"
#include "itkScaleLogarithmicTransform.h"
#include "itkQuaternionRigidTransform.h"
#include "itkVersorTransform.h"
#include "itkAffineTransform.h"
#include "itkCompositeTransform.h"

#include <memory>

namespace itk
{
namespace simple
{

class PimpleTransformBase
{
public:
  virtual ~PimpleTransformBase( void ) {};

  virtual TransformBase::Pointer GetTransformBase( void ) = 0;
  virtual TransformBase::ConstPointer GetTransformBase( void ) const = 0;

  virtual unsigned int GetInputDimension( void ) const = 0;
  virtual unsigned int GetOutputDimension( void ) const = 0;

  void SetFixedParameters( const std::vector< double > &inParams )
    {
      itk::TransformBase::ParametersType p( inParams.size() );

      // todo check expected number of Fixed parameters
      std::copy( inParams.begin(), inParams.end(), p.begin() );
      this->GetTransformBase()->SetFixedParameters( p );
    }
  std::vector< double >  GetFixedParameters( void ) const
    {
      const itk::TransformBase::ParametersType &p = this->GetTransformBase()->GetFixedParameters();
      return std::vector< double >( p.begin(), p.end() );
    }

  unsigned int GetNumberOfParameters( void ) const { return this->GetTransformBase()->GetNumberOfParameters(); }


  void SetParameters( const std::vector< double > &inParams )
    {
      itk::TransformBase::ParametersType p( inParams.size() );

      // todo check expected number of Parameters
      std::copy( inParams.begin(), inParams.end(), p.begin() );
      this->GetTransformBase()->SetParameters( p );
    }
  std::vector< double > GetParameters( void ) const
    {
      const itk::TransformBase::ParametersType &p = this->GetTransformBase()->GetParameters();
      return std::vector< double >( p.begin(), p.end() );
    }


  virtual PimpleTransformBase *ShallowCopy( void ) const = 0;
  virtual PimpleTransformBase *DeepCopy( void ) const = 0;

  virtual int GetReferenceCount( ) const = 0;

  std::string ToString( void ) const
    {
      std::ostringstream out;
      this->GetTransformBase()->Print ( out );
      return out.str();
    }


protected:

};

template< typename TTransformType >
class PimpleTransform
  : public PimpleTransformBase
{
public:
  typedef PimpleTransform                  Self;
  typedef TTransformType                   TransformType;
  typedef typename TransformType::Pointer  TransformPointer;

  static const unsigned int InputDimension = TTransformType::InputSpaceDimension;
  static const unsigned int OutputDimension = TTransformType::OutputSpaceDimension;

  PimpleTransform( TransformType * p)
    {
      this->m_Transform = p;
    }

  PimpleTransform( )
    {
      this->m_Transform = TransformType::New();
    }

  virtual TransformBase::Pointer GetTransformBase( void ) { return this->m_Transform.GetPointer(); }
  virtual TransformBase::ConstPointer GetTransformBase( void ) const { return this->m_Transform.GetPointer(); }

  virtual unsigned int GetInputDimension( void ) const { return InputDimension; }
  virtual unsigned int GetOutputDimension( void ) const { return OutputDimension; }


  virtual PimpleTransformBase *ShallowCopy( void ) const
    {
      return new Self( this->m_Transform.GetPointer() );
    }

  virtual PimpleTransformBase *DeepCopy( void ) const
    {
      std::auto_ptr<Self> copy( new Self() );
      copy->m_Transform->SetFixedParameters( this->m_Transform->GetFixedParameters() );
      copy->m_Transform->SetParameters( this->m_Transform->GetParameters() );
      return copy.release();
    }

  virtual int GetReferenceCount( ) const
    {
      return this->m_Transform->GetReferenceCount();
    }

private:

  TransformPointer m_Transform;
};

Transform::Transform( )
  : m_PimpleTransform( NULL )
  {
    m_PimpleTransform = new PimpleTransform<itk::IdentityTransform< double, 3 > >();
  }

  Transform::Transform( unsigned int dimensions, TransformEnum type)
    : m_PimpleTransform( NULL )
  {
    if ( dimensions == 2 )
      {
      this->InternalIntitialization<2 >(type);
      }
    else if ( dimensions == 3 )
      {
      this->InternalIntitialization<3>(type);
      }
    else
      {
      sitkExceptionMacro("Invalid dimension for transform");
      }
  }

  Transform::~Transform()
  {
    delete m_PimpleTransform;
    this->m_PimpleTransform = NULL;
  }

  Transform::Transform( const Transform &txf )
    : m_PimpleTransform( NULL )
  {
    m_PimpleTransform = txf.m_PimpleTransform->ShallowCopy();
  }

  Transform& Transform::operator=( const Transform & txf )
  {
    // note: if txf and this are the same,the following statements
    // will be safe. It's also exception safe.
    std::auto_ptr<PimpleTransformBase> temp( txf.m_PimpleTransform->ShallowCopy() );
    delete this->m_PimpleTransform;
    this->m_PimpleTransform = temp.release();
    return *this;
  }

void Transform::MakeUniqueForWrite( void )
{
  if ( this->m_PimpleTransform->GetReferenceCount() > 1 )
    {
    std::auto_ptr<PimpleTransformBase> temp( this->m_PimpleTransform->DeepCopy() );
    delete this->m_PimpleTransform;
    this->m_PimpleTransform = temp.release();
    }
}

  template< unsigned int VDimension>
  void  Transform::InternalIntitialization(  TransformEnum type, itk::TransformBase *base )
  {
    switch( type )
      {

      case sitkTranslation:
        m_PimpleTransform = new PimpleTransform<itk::TranslationTransform< double, VDimension > >();
        break;
      case sitkScale:
        m_PimpleTransform = new PimpleTransform<itk::ScaleTransform< double, VDimension > >();
        break;
      case sitkScaleLogarithmic:
        m_PimpleTransform = new PimpleTransform<itk::ScaleLogarithmicTransform< double, VDimension > >();
        break;
      case sitkQuaternionRigid:
        // todo what todo about transform which require a specific dimension
        assert( VDimension == 3 );
        m_PimpleTransform = new PimpleTransform<itk::QuaternionRigidTransform< double > >();
        break;
      case sitkVersor:
        // todo what todo about transform which require a specific dimension
        assert( VDimension == 3 );
        m_PimpleTransform = new PimpleTransform<itk::VersorTransform< double > >();
        break;
      case sitkAffine:
        m_PimpleTransform = new PimpleTransform<itk::AffineTransform< double, VDimension > >();
        break;
      case sitkComposite:
      {
      if ( !base )
        {
        m_PimpleTransform = new PimpleTransform<itk::CompositeTransform<double, VDimension> > ();
        break;
        }
      itk::CompositeTransform<double, VDimension>* compositeTransform =
        dynamic_cast<itk::CompositeTransform<double, VDimension>*>( base );
      if ( !compositeTransform )
        {
        sitkExceptionMacro("Unexpectedly unable to convert to CompositeTransform" );
        }
      m_PimpleTransform = new PimpleTransform<itk::CompositeTransform<double, VDimension> >( compositeTransform );
      }
      break;
      case sitkIdentity:
      default:
        m_PimpleTransform = new PimpleTransform<itk::IdentityTransform< double, VDimension > >();

      }
  }


  itk::TransformBase* Transform::GetITKBase ( void )
  {
    assert( m_PimpleTransform );
    return this->m_PimpleTransform->GetTransformBase();
  }

  const itk::TransformBase* Transform::GetITKBase ( void ) const
  {
    assert( m_PimpleTransform );
    return this->m_PimpleTransform->GetTransformBase();
  }

  unsigned int  Transform::GetDimension( void ) const
  {
    assert( m_PimpleTransform );
    return this->m_PimpleTransform->GetInputDimension();
  }

  void Transform::SetParameters ( const std::vector<double>& parameters )
  {
    assert( m_PimpleTransform );
    this->MakeUniqueForWrite();
    this->m_PimpleTransform->SetParameters( parameters );
  }
  std::vector<double> Transform::GetParameters( void ) const
  {
    assert( m_PimpleTransform );
    return this->m_PimpleTransform->GetParameters();
  }


  std::string Transform::ToString( void ) const
  {
    assert( m_PimpleTransform );
    return this->m_PimpleTransform->ToString();
  }
}
}
