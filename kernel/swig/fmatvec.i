// check target language
//MFMF move to swig module
#ifndef SWIGPYTHON
  #error "Only Pyhton as target language is supported."
#endif

// add code to the generated code
%{

#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION
#include <numpy/arrayobject.h>

#include <typeinfo>
#include <fmatvec/atom.h>
#include <fmatvec/range.h>

template<typename AT> void _checkNumPyType(int type);
template<> void _checkNumPyType<int>(int type) {
  if(type!=NPY_SHORT    && type!=NPY_USHORT    &&
     type!=NPY_INT      && type!=NPY_UINT      &&
     type!=NPY_LONG     && type!=NPY_ULONG     &&
     type!=NPY_LONGLONG && type!=NPY_ULONGLONG)
    throw std::runtime_error("Value is not of type integer.");
}
template<> void _checkNumPyType<double>(int type) {
  if(type!=NPY_SHORT    && type!=NPY_USHORT    &&
     type!=NPY_INT      && type!=NPY_UINT      &&
     type!=NPY_LONG     && type!=NPY_ULONG     &&
     type!=NPY_LONGLONG && type!=NPY_ULONGLONG &&
     type!=NPY_FLOAT    && type!=NPY_DOUBLE    && type!=NPY_LONGDOUBLE)
    throw std::runtime_error("Value is not of type floating point.");
}

template<typename AT> AT _arrayGet(PyArrayObject *a, int type, int r, int c=-1);
template<> int _arrayGet<int>(PyArrayObject *a, int type, int r, int c) {
  switch(type) {
    case NPY_SHORT:      return *static_cast<npy_short*>     (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_USHORT:     return *static_cast<npy_ushort*>    (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_INT:        return *static_cast<npy_int*>       (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_UINT:       return *static_cast<npy_uint*>      (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_LONG:       return *static_cast<npy_long*>      (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_ULONG:      return *static_cast<npy_ulong*>     (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_LONGLONG:   return *static_cast<npy_longlong*>  (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_ULONGLONG:  return *static_cast<npy_ulonglong*> (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
  }
  throw std::runtime_error("Value is not of type floating point (wrong element type).");
}
template<> double _arrayGet<double>(PyArrayObject *a, int type, int r, int c) {
  switch(type) {
    case NPY_SHORT:      return *static_cast<npy_short*>     (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_USHORT:     return *static_cast<npy_ushort*>    (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_INT:        return *static_cast<npy_int*>       (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_UINT:       return *static_cast<npy_uint*>      (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_LONG:       return *static_cast<npy_long*>      (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_ULONG:      return *static_cast<npy_ulong*>     (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_LONGLONG:   return *static_cast<npy_longlong*>  (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_ULONGLONG:  return *static_cast<npy_ulonglong*> (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_FLOAT:      return *static_cast<npy_float*>     (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_DOUBLE:     return *static_cast<npy_double*>    (c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
    case NPY_LONGDOUBLE: return *static_cast<npy_longdouble*>(c==-1 ? PyArray_GETPTR1(a, r) : PyArray_GETPTR2(a, r, c));
  }
  throw std::runtime_error("Value is not of type floating point (wrong element type).");
}

template<class T> inline T& _derefIfPointer(T & t) { return  t; }
template<class T> inline T& _derefIfPointer(T*& t) { return *t; }

template<class T> inline void _assignIfPointer(T & d, T& s) {}
template<class T> inline void _assignIfPointer(T*& d, T& s) { d=&s; }

template<class T> inline void _assignOrCopy(T & d, T* s) { d=*s; }
template<class T> inline void _assignOrCopy(T*& d, T* s) { d= s; }

template<typename AT> constexpr int _numPyType();
template<> constexpr int _numPyType<int>() { return NPY_LONG; }
template<> constexpr int _numPyType<double>() { return NPY_DOUBLE; }



template<typename Vec>
void _typemapOutVec_P_R(Vec *_1, PyObject *&_result) {
  npy_intp dims[1];
  dims[0]=_1->size();
  _result=PyArray_SimpleNewFromData(1, dims, _numPyType<typename Vec::AtomicType>(), &(*_1)(0));
  if(!_result)
    throw std::runtime_error("Cannot create ndarray");
}

template<typename Vec>
void _typemapOutVec_V_CR(Vec &_1, PyObject *&_result) {
  typedef typename std::remove_pointer<Vec>::type VecBT;
  npy_intp dims[1];
  dims[0]=_derefIfPointer(_1).size();
  _result=PyArray_SimpleNew(1, dims, _numPyType<typename VecBT::AtomicType>());
  if(!_result)
    throw std::runtime_error("Cannot create ndarray");
  std::copy(&_derefIfPointer(_1)(0), &_derefIfPointer(_1)(0)+_derefIfPointer(_1).size(),
            static_cast<typename VecBT::AtomicType*>(PyArray_GETPTR1(reinterpret_cast<PyArrayObject*>(_result), 0)));
}

template<typename Vec>
void _typemapInVec_P_R_V(Vec &_1, PyObject *_input, typename std::remove_pointer<Vec>::type &localVar, swig_type_info *_1_descriptor) {
  typedef typename std::remove_pointer<Vec>::type VecBT;
  void *inputp;
  int res=SWIG_ConvertPtr(_input, &inputp, _1_descriptor, 0);
  if(SWIG_IsOK(res))
    _assignOrCopy(_1, reinterpret_cast<VecBT*>(inputp));
  else if(PyArray_Check(_input)) {
    PyArrayObject *input=reinterpret_cast<PyArrayObject*>(_input);
    if(PyArray_NDIM(input)!=1)
      throw std::runtime_error("Must have 1 dimension.");
    int type=PyArray_TYPE(input);
    _checkNumPyType<typename VecBT::AtomicType>(type);
    _assignIfPointer(_1, localVar);
    npy_intp *dims=PyArray_SHAPE(input);
    _derefIfPointer(_1).resize(dims[0]);
    for(int i=0; i<dims[0]; ++i)
      _derefIfPointer(_1)(i)=_arrayGet<typename VecBT::AtomicType>(input, type, i);
  }
  else
    throw std::runtime_error("Wrong type.");
}

template<typename Vec>
void _typemapArgoutVec_R(PyObject *_input, typename std::remove_reference<Vec>::type &localVar) {
  typedef typename std::remove_reference<Vec>::type VecBT;
  PyArrayObject *input=reinterpret_cast<PyArrayObject*>(_input);
  int type=PyArray_TYPE(input);
  if(type!=_numPyType<typename VecBT::AtomicType>())
    throw std::runtime_error(std::string("Must have atomic type ")+typeid(typename VecBT::AtomicType).name());
  std::copy(&localVar(0), &localVar(0)+localVar.size(),
            static_cast<typename VecBT::AtomicType*>(PyArray_GETPTR1(input, 0)));
}

template<typename Mat>
void _typemapOutMat_P_R(Mat *_1, PyObject *&_result) {
  npy_intp dims[2];
  dims[0]=_1->rows();
  dims[1]=_1->cols();
  _result=PyArray_SimpleNewFromData(2, dims, _numPyType<typename Mat::AtomicType>(), &(*_1)(0, 0));
  if(!_result)
    throw std::runtime_error("Cannot create ndarray");
  npy_intp *strides=PyArray_STRIDES(reinterpret_cast<PyArrayObject*>(_result));
  strides[_1->transposed() ? 1 : 0]=sizeof(typename Mat::AtomicType)*1;
  strides[_1->transposed() ? 0 : 1]=sizeof(typename Mat::AtomicType)*_1->ldim();
}

template<typename Mat>
void _typemapOutMat_V_CR(Mat &_1, PyObject *&_result) {
  typedef typename std::remove_pointer<Mat>::type MatBT;
  npy_intp dims[2];
  dims[0]=_derefIfPointer(_1).rows();
  dims[1]=_derefIfPointer(_1).cols();
  _result=PyArray_SimpleNew(2, dims, _numPyType<typename MatBT::AtomicType>());
  if(!_result)
    throw std::runtime_error("Cannot create ndarray");
  PyArrayObject *input=reinterpret_cast<PyArrayObject*>(_result);
  for(int r=0; r<dims[0]; r++)
    for(int c=0; c<dims[1]; c++)
      *static_cast<typename MatBT::AtomicType*>(PyArray_GETPTR2(input, r, c))=
        _derefIfPointer(_1)(r, c);
}

template<typename Mat>
void _typemapInMat_P_R_V(Mat &_1, PyObject *_input, typename std::remove_pointer<Mat>::type &localVar, swig_type_info *_1_descriptor) {
  typedef typename std::remove_pointer<Mat>::type MatBT;
  void *inputp;
  int res=SWIG_ConvertPtr(_input, &inputp, _1_descriptor, 0);
  if(SWIG_IsOK(res))
    _assignOrCopy(_1, reinterpret_cast<MatBT*>(inputp));
  else if(PyArray_Check(_input)) {
    PyArrayObject *input=reinterpret_cast<PyArrayObject*>(_input);
    if(PyArray_NDIM(input)!=2)
      throw std::runtime_error("Must have 2 dimension.");
    int type=PyArray_TYPE(input);
    _checkNumPyType<typename MatBT::AtomicType>(type);
    _assignIfPointer(_1, localVar);
    npy_intp *dims=PyArray_SHAPE(input);
    _derefIfPointer(_1).resize(dims[0], dims[1]);
    for(int r=0; r<dims[0]; ++r)
      for(int c=0; c<dims[1]; ++c)
        _derefIfPointer(_1)(r, c)=_arrayGet<typename MatBT::AtomicType>(input, type, r, c);
  }
  else
    throw std::runtime_error("Wrong type.");
}

template<typename Mat>
void _typemapArgoutMat_R(PyObject *_input, typename std::remove_reference<Mat>::type &localVar) {
  typedef typename std::remove_reference<Mat>::type MatBT;
  PyArrayObject *input=reinterpret_cast<PyArrayObject*>(_input);
  int type=PyArray_TYPE(input);
  if(type!=_numPyType<typename MatBT::AtomicType>())
    throw std::runtime_error(std::string("Must have atomic type ")+typeid(typename MatBT::AtomicType).name());
  for(int r=0; r<localVar.rows(); ++r)
    for(int c=0; c<localVar.cols(); ++c)
      *static_cast<typename MatBT::AtomicType*>(PyArray_GETPTR2(input, r, c))=
        localVar(r, c);
}

%}

// init numpy
%init %{
  _import_array();
%}

%import "std_string.i"

// use SWIG_exception to throw a target language exception
%include exception.i

// helper define to convert c++ exceptions in typemaps to target language exceptions
%define FMATVEC_CATCHARG
  catch(const std::exception &ex) {
    Py_XDECREF($result);
    SWIG_exception(SWIG_RuntimeError, (std::string("In function $symname, argument $argnum: ")+ex.what()).c_str());
  }
%enddef

// fmatvec typedefs and template instantiations
%include <fmatvec/fmatvec.h>

%template() fmatvec::Vector   <fmatvec::Ref      , int   >;
%template() fmatvec::Vector   <fmatvec::Var      , int   >;
%template() fmatvec::Vector   <fmatvec::Fixed< 1>, int   >;
%template() fmatvec::Vector   <fmatvec::Fixed< 2>, int   >;
%template() fmatvec::Vector   <fmatvec::Fixed< 3>, int   >;
%template() fmatvec::Vector   <fmatvec::Ref      , double>;
%template() fmatvec::Vector   <fmatvec::Var      , double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 1>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 2>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 3>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 4>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 5>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 6>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 7>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 8>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed< 9>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<10>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<11>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<12>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<13>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<14>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<15>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<16>, double>;
%template() fmatvec::Vector   <fmatvec::Fixed<17>, double>;
%template() fmatvec::RowVector<fmatvec::Var      , int   >;
%template() fmatvec::RowVector<fmatvec::Ref      , double>;
%template() fmatvec::RowVector<fmatvec::Var      , double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 1>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 2>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 3>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 4>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 5>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 6>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 7>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 8>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed< 9>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<10>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<11>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<12>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<13>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<14>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<15>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<16>, double>;
%template() fmatvec::RowVector<fmatvec::Fixed<17>, double>;

%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Var      , int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 1>, int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 2>, int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 3>, int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 4>, int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Var      , int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Var      , int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Var      , int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Var      , int   >;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Ref      , fmatvec::Ref      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Var      , fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 1>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 2>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 3>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 4>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 5>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 6>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 7>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 8>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed< 9>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<10>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<11>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<12>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<13>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<14>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<15>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<16>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 5>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 6>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 7>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 8>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed< 9>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<10>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<11>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<12>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<13>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<14>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<15>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<16>, double>;
%template() fmatvec::Matrix<fmatvec::General    , fmatvec::Fixed<17>, fmatvec::Fixed<17>, double>;
%template() fmatvec::Matrix<fmatvec::Rotation   , fmatvec::Fixed< 3>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Ref      , fmatvec::Ref      , double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Var      , fmatvec::Var      , double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Fixed< 1>, fmatvec::Fixed< 1>, double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Fixed< 2>, fmatvec::Fixed< 2>, double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Fixed< 3>, fmatvec::Fixed< 3>, double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Fixed< 4>, fmatvec::Fixed< 4>, double>;
%template() fmatvec::Matrix<fmatvec::Symmetric  , fmatvec::Fixed< 5>, fmatvec::Fixed< 5>, double>;

//MFMF not working %template() fmatvec::Matrix<fmatvec::Sparse     , fmatvec::Ref      , fmatvec::Ref      , double>;
//MFMF not working %template() fmatvec::Matrix<fmatvec::Diagonal   , fmatvec::Ref      , fmatvec::Ref      , double>;
//MFMF not working %template() fmatvec::Matrix<fmatvec::GeneralBand, fmatvec::Ref      , fmatvec::Ref      , double>;

%template() fmatvec::SquareMatrix<fmatvec::Ref      , double>;
%template() fmatvec::SquareMatrix<fmatvec::Var      , double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 1>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 2>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 3>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 4>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 5>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 6>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 7>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 8>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed< 9>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<10>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<11>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<12>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<13>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<14>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<15>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<16>, double>;
%template() fmatvec::SquareMatrix<fmatvec::Fixed<17>, double>;

// wrap the following
%include <fmatvec/range.h>
%template(Index) fmatvec::Range<fmatvec::Var,fmatvec::Var>;



// wrap output (member of function return value) as a numpy.ndarray which uses the date memory from the c++ member
%typemap(out, noblock=1) fmatvec::Vector*,
                         fmatvec::Vector&,
                         fmatvec::RowVector*,
                         fmatvec::RowVector& {
  try {
    _typemapOutVec_P_R($1, $result);
  }
  FMATVEC_CATCHARG
}

// wrap output (function return value) as a numpy.ndarray with its own data memory
%typemap(out, noblock=1) fmatvec::Vector,
                         const fmatvec::Vector&,
                         fmatvec::RowVector,
                         const fmatvec::RowVector& {
  try {
    _typemapOutVec_V_CR($1, $result);
  }
  FMATVEC_CATCHARG
}

// add a local variable
// (we cannot use the SWIG syntax "typemap(...) TYPE (LocalVarType localVar)" here since LocalVarType depends on the template type)
%typemap(arginit, noblock=1) fmatvec::Vector*,
                             fmatvec::Vector&,
                             fmatvec::Vector,
                             fmatvec::RowVector*,
                             fmatvec::RowVector&,
                             fmatvec::RowVector {
  $1_basetype localVar$argnum;
}

// wrap input (member or function parameter) as a numpy.ndarray with its own data memory
%typemap(in, noblock=1) fmatvec::Vector*,
                        fmatvec::Vector&,
                        fmatvec::Vector,
                        fmatvec::RowVector*,
                        fmatvec::RowVector&,
                        fmatvec::RowVector {
  try {
    _typemapInVec_P_R_V($1, $input, localVar$argnum, $1_descriptor);
  }
  FMATVEC_CATCHARG
}

// wrap output function parameter -> do nothing for const references (no output)
%typemap(argout, noblock=1) const fmatvec::Vector&,
                            const fmatvec::RowVector& {
}

// wrap output function parameter -> copy result back to input function parameter
%typemap(argout, noblock=1) fmatvec::Vector&,
                            fmatvec::RowVector& {
  try {
    _typemapArgoutVec_R<$1_type>($input, localVar$argnum);
  }
  FMATVEC_CATCHARG
}



// wrap output (member of function return value) as a numpy.ndarray which uses the date memory from the c++ member
%typemap(out, noblock=1) fmatvec::Matrix*,
                         fmatvec::Matrix&,
                         fmatvec::SquareMatrix*,
                         fmatvec::SquareMatrix& {
  try {
    _typemapOutMat_P_R($1, $result);
  }
  FMATVEC_CATCHARG
}

// wrap output (function return value) as a numpy.ndarray with its own data memory
%typemap(out, noblock=1) fmatvec::Matrix,
                         const fmatvec::Matrix&,
                         fmatvec::SquareMatrix,
                         const fmatvec::SquareMatrix& {
  try {
    _typemapOutMat_V_CR($1, $result);
  }
  FMATVEC_CATCHARG
}

// add a local variable
// (we cannot use the SWIG syntax "typemap(...) TYPE (LocalVarType localVar)" here since LocalVarType depends on the template type)
%typemap(arginit, noblock=1) fmatvec::Matrix*,
                             fmatvec::Matrix&,
                             fmatvec::Matrix,
                             fmatvec::SquareMatrix*,
                             fmatvec::SquareMatrix&,
                             fmatvec::SquareMatrix {
  $1_basetype localVar$argnum;
}

// wrap input (member or function parameter) as a numpy.ndarray with its own data memory
%typemap(in, noblock=1) fmatvec::Matrix*,
                        fmatvec::Matrix&,
                        fmatvec::Matrix,
                        fmatvec::SquareMatrix*,
                        fmatvec::SquareMatrix&,
                        fmatvec::SquareMatrix {
  try {
    _typemapInMat_P_R_V($1, $input, localVar$argnum, $1_descriptor);
  }
  FMATVEC_CATCHARG
}

// wrap output function parameter -> do nothing for const references (no output)
%typemap(argout, noblock=1) const fmatvec::Matrix&,
                            const fmatvec::SquareMatrix& {
}

// wrap output function parameter -> copy result back to input function parameter
%typemap(argout, noblock=1) fmatvec::Matrix&,
                            fmatvec::SquareMatrix& {
  try {
    _typemapArgoutMat_R<$1_type>($input, localVar$argnum);
  }
  FMATVEC_CATCHARG
}



// wrap fmatvec::Atom
%feature("director") fmatvec::Atom;
namespace fmatvec {
  %extend Atom {
    void msg(MsgType type, const std::string &msg) {
      $self->msg(type)<<msg<<std::endl;
    }
    static void msgStatic(MsgType type, const std::string &msg) {
      fmatvec::Atom::msgStatic(type)<<msg<<std::endl;
    }
  };
}
%ignore fmatvec::Atom::setCurrentMessageStream;
%ignore fmatvec::Atom::setMessageStreamActive;
%ignore fmatvec::Atom::getMessageStream;
%ignore fmatvec::Atom::adoptMessageStreams;
%ignore fmatvec::Atom::msg;
%ignore fmatvec::Atom::msgStatic;
%include <fmatvec/atom.h>
