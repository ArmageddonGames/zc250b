
#ifndef __zc_array_h_
#define __zc_array_h_

#define _DEBUGZCARRAY
#ifdef _DEBUGZCARRAY
#include "zdefs.h"
#endif

template <typename T>
class ZCArray
{
public:
	typedef unsigned int size_type;
	typedef const T& const_reference;
	typedef const T* const_pointer;
	typedef const T const_type;
	typedef T& reference;
	typedef T* pointer;
	typedef T type;

	ZCArray() : _ptr(NULL), _size(NULL)
		{
			for(char i = 0; i < 4; i++)
				_dim[i] = 0;
		}

	ZCArray( const size_type& _Size ) : _ptr(NULL)
		{
			_SetDimensions( 0, 0, _Size );
			_Alloc(_size);
		}

	ZCArray( const size_type& _Y, const size_type& _X ) : _ptr(NULL)
		{
			_SetDimensions( 0, _Y, _X );
			_Alloc(_size);
		}

	ZCArray( const size_type& _Z, const size_type& _Y, const size_type& _X ) : _ptr(NULL)
		{
			_SetDimensions( _Z, _Y, _X );
			_Alloc(_size);
		}

	ZCArray( const ZCArray &_Array ) : _ptr(NULL)
		{
			Copy(_Array);
		}

	~ZCArray()
		{
			_Delete();
		}

	const ZCArray &operator = ( const ZCArray &_Array )
		{
			if ( this != &_Array )
				Copy(_Array);

			return *this;
		}

	bool operator == ( const ZCArray &_Array ) const
		{
			if ( _size != _Array._size )
				return false;

			for( size_type i(0); i < _size; i++ )
				if( *(_ptr + i) != *(_Array._ptr + i) )
					return false;

			return true;
		}

	bool operator != ( const ZCArray &right ) const { return !( *this == right ); }

	reference operator () ( size_type _X )								{ return _ptr[ _X ]; }
	reference operator () ( size_type _Y, size_type _X )				{ return _ptr[ _X + _Y * _dim[0] ]; }
	reference operator () ( size_type _Z, size_type _Y, size_type _X )	{ return _ptr[ _X + _Y * _dim[0] + _Z * _dim[3] ]; }

	const_reference operator () ( size_type _X ) const								{ return _ptr[ _X ]; }
	const_reference operator () ( size_type _Y, size_type _X ) const				{ return _ptr[ _X + _Y * _dim[0] ]; }
	const_reference operator () ( size_type _Z, size_type _Y, size_type _X ) const	{ return _ptr[ _X + _Y * _dim[0] + _Z * _dim[3] ]; }

	pointer		operator ->()		{ return &(*_ptr); }
	const_pointer	operator ->() const	{ return &(*_ptr); }
	type			operator * ()		{ return *_ptr; }
	const_type	operator * () const	{ return *_ptr; }
	reference		operator []	( const size_type& i )
		{
			reference ret = _ptr[i];
#ifdef _DEBUGZCARRAY
			al_trace("offset: %i\n");
			al_trace("return value: %i\n", ret);
#endif
			return ret;
		}
	const_reference operator [] ( const size_type& i ) const	{ return _ptr[i]; }

	reference At( const size_type& _X )									{ _Xran(_X); return _ptr[ _X ]; }
	reference At( const size_type& _Y, const size_type& _X )					{ _Xran(_X,_Y); return _ptr[ Offset(_Y, _X) ]; }
	reference At( const size_type& _Z, const size_type& _Y, const size_type& _X )	{ _Xran(_X,_Y,_Z); return _ptr[ Offset(_Z, _Y, _X) ]; }

	reference Front() { return *_ptr; }
	reference Back () { return *(_ptr + (_size - 1)); }
	const_reference Front() const { return *_ptr; }
	const_reference Back () const { return *(_ptr + (_size - 1)); }

	size_type Offset( const size_type& _Z, const size_type& _Y, const size_type& _X ) const { return (_X + _Y * _dim[0] + _Z * _dim[3]); }
	size_type Offset( const size_type& _Y, const size_type& _X ) const	{ return (_X + _Y * _dim[0]); }

	size_type Size() const			{ return _size;		}

	bool Empty() const { return (_size == 0); }

	void Assign( const size_type& _Begin, const size_type& _End, const type& _Val = type() )
		{
			for( size_type i(_Begin); i < _End; i++ )
				_ptr[ i ] = _Val;
		}

	void Resize( const size_type& _Size )				{ Resize( 0, 0, _Size ); }
	void Resize( const size_type& _Y, const size_type& _X )	{ Resize( 0, _Y, _X ); }
	void Resize( const size_type& _Z, const size_type& _Y, const size_type& _X )
		{
			const size_type _OldSize = _size;
			const size_type _NewSize = _GetSize( _Z, _Y, _X );

			_SetDimensions( _Z, _Y, _X );

			if( _NewSize == 0 )
				_Delete();
			else if( _OldSize != _NewSize )
				_ReAssign( _OldSize, _NewSize );
		}

	void Copy( const ZCArray &_Array )
		{
			if( _Array.Size() == 0 )
			{
				_Delete();
				_size = 0;
				for(char i = 0; i < 4; i++)
					_dim[i] = 0;
				return;
			}

			if( _size != _Array.Size() )
				_Alloc( _Array.Size() );

			_SetDimensions( _Array._dim[2], _Array._dim[1], _Array._dim[0] );

			for( size_type i(0); i < _size; i++ )
				_ptr[ i ] = _Array._ptr[ i ];
		}

	void Clone( ZCArray& _RefArray ) const
		{
			_RefArray = *this;
		}

	void GetDimensions( size_type _4dim[] ) const
		{
			_4dim[0] = _dim[0]; _4dim[1] = _dim[1];
			_4dim[2] = _dim[2]; _4dim[3] = _dim[3];
		}

	void Clear()
		{
			_Delete();
			_size = 0;
		}

    void Allocate( const size_type& size)
        {
            _Alloc(size);
        }

protected:

	void _Alloc( const size_type& size )
		{

#ifdef _DEBUGZCARRAY
			al_trace("Memory to allocate: %i\n", size);
#endif
			if(_ptr)
				_Delete();
			if( size == 0 )
				throw ("Cannot allocate a zero sized Array.");
			_ptr = new type[ size ];
			_size = size;
		}

	void _ReAssign( const size_type& _OldSize, const size_type& _NewSize )
		{
			pointer _oldPtr = _ptr;
			_ptr = new type[ _NewSize ];

			const size_type _copyRange = (_OldSize < _NewSize ? _OldSize : _NewSize);

			for( size_type i(0); i < _copyRange; i++ )
				_ptr[ i ] = _oldPtr[ i ];

			_Delete(_oldPtr);
			_size = _NewSize;
		}

	void _Delete()
		{
			if(_ptr)
				delete [] _ptr;
			_ptr = NULL;
		}

	void _Delete( pointer _Ptr )
		{
			if(_Ptr)
				delete [] _Ptr;
			_Ptr = NULL;
		}

	void _SetDimensions( size_type _Z, size_type _Y, size_type _X )
		{
			_dim[0] = _X; _dim[1] = _Y; _dim[2] = _Z;
			_dim[3] = (_X * _Y);

			_size = _GetSize( _X, _Y, _Z );
		}

	size_type _GetSize( size_type _Z, size_type _Y, size_type _X ) const
		{
			if( _Z > 0 )
				return (_X * _Y * _Z);
			else if( _Y > 0 )
				return (_X * _Y);
			return (_X);
		}

	void _Xran( size_type _X )
		{
			if( _X >= _size )
				Resize(_X);
		}

	void _Xran( size_type _X, size_type _Y ) const
		{
			if( Offset(_Y, _X) >= _size )
				throw ("Array indices out of range.");
		}

	void _Xran( size_type _X, size_type _Y, size_type _Z ) const
		{
			if( Offset(_Z, _Y, _X) >= _size )
				throw ("Array indices out of range.");
		}

private:
	size_type _size;
	size_type _dim[ 4 ];
	pointer _ptr;

};


#endif

