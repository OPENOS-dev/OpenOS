/*
Canon sane driver is licensed under the Apache License 2.0.
See the LICENSE file for more details.
*/
#ifndef __CEI_UNKNOWN_INTERFACE_HEADER_DEFINED__
#define __CEI_UNKNOWN_INTERFACE_HEADER_DEFINED__

#ifdef _WIN32
#include <Unknwn.h>
#else
typedef long REFIID;
#define STDMETHODCALLTYPE
class IUnknown
{
public:
	virtual long STDMETHODCALLTYPE QueryInterface(REFIID id, void **ppOut)=0;
	virtual unsigned long STDMETHODCALLTYPE AddRef()=0;
	virtual unsigned long STDMETHODCALLTYPE Release()=0;
};

#endif

#ifndef _XINTERFACE_DEFINED_
#define _XINTERFACE_DEFINED_
template<class T> class XInterface
{
public:
    XInterface( T * p = 0 ) : _p( p ) {}
	XInterface( XInterface<T> &p ) {
		_p=p;
		if (_p) _p->AddRef();
	}
    ~XInterface() { if ( 0 != _p ) _p->Release(); }
    T * operator->() { return _p; }
    T * GetPointer() const { return _p; }
    T *get(){return GetPointer();}
    IUnknown ** GetIUPointer() { return (IUnknown **) &_p; }
    T ** GetPPointer() { 
		if ( 0 != _p) _p->Release();
		_p=NULL;
		return &_p; 
	}
	T** operator &() {
		return GetPPointer();
	}
	operator T** () {
		return GetPPointer();
	}
	operator T*() {
		return GetPointer();
	}
	
    void ** GetQIPointer() { return (void **) &_p; }
    T * Acquire() { T * p = _p; _p = 0; return p; }
    bool IsNull() { return ( 0 == _p ); }
	T* operator =(T* p) { 
		if ( 0 != _p) _p->Release();
		_p = p;
		return _p;
	}
	T *Detach() {
		T *p=_p;
		_p=NULL;
		return p;
	}
	XInterface<T> &operator = (XInterface<T> &in) 
	{
		if ( 0 != _p) _p->Release();
		_p = in;
		if (_p) _p->AddRef();
		return *this;
	}
    void reset(T *p) {
    	*this = p;
    }	
private:
    T * _p;
};
#endif //_XINTERFACE_DEFINED_

#endif