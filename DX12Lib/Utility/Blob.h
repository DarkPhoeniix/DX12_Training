#pragma once

#include <stdio.h>
#include <cstring>

#pragma warning(disable : 4995 4996 26439)

namespace Base
{
	class Blob
	{
	public:
		// default {ctor} and {dtor}
		//
		Blob()
			: _pPtr( nullptr )
			, _uiAllocatedSize( 0 )
			, _isOwner( true )
		{
		}

		virtual ~Blob()
		{
			this->clean();
		}

		// ctor with nullptr
		//
		Blob( std::nullptr_t )
			: _pPtr( nullptr )
			, _uiAllocatedSize( 0 )
			, _isOwner( true )
		{
		}

		// copy\move and assigment
		//
		Blob( const Blob& other )
			: _pPtr( nullptr )
			, _uiAllocatedSize( 0 )
			, _isOwner( true )
		{
			if( !other._isOwner )
			{
				this->_isOwner = other._isOwner;
				this->_pPtr = other._pPtr;
				this->_uiAllocatedSize = other._uiAllocatedSize;
			}
			else
			{
				this->__Copy( other );
			}
		}

		Blob& operator=( const Blob& other )
		{
			if( this != &other )
			{
				if( !other._isOwner )
				{
					this->_isOwner = other._isOwner;
					this->_pPtr = other._pPtr;
					this->_uiAllocatedSize = other._uiAllocatedSize;
				}
				else
				{
					this->clean();
					this->__Copy( other );
				}
			}
			return *this;
		}

		Blob( Blob&& other )
			: _pPtr( other._pPtr )
			, _uiAllocatedSize( other._uiAllocatedSize )
			, _isOwner( other._isOwner )
		{
			this->__Move( std::move( other ) );
		}

		Blob& operator=( Blob&& other )
		{
			if( this != &other )
			{
				this->clean();
				this->__Move( std::move( other ) );
			}
			return *this;
		}

		// clean block for assigment nullptr
		//
		inline void operator=( decltype( nullptr ) )
		{
			this->clean();
		}

	private:
		void __Copy( const Blob& other )
		{
			//this->allocate( other._uiAllocatedSize );
			if( other.empty() )
			{
				return;
			}

			// copy blob data
			//
			this->set( other._pPtr, other._uiAllocatedSize );
		}

		void __Move( Blob&& other )
		{
			this->_pPtr = other._pPtr;
			this->_uiAllocatedSize = other._uiAllocatedSize;

			this->_pPtr = nullptr;
			other._uiAllocatedSize = 0;

			this->_isOwner = other._isOwner;

		}
	public:
		// comparing
		inline bool operator==( const Blob& other ) const
		{
			return this->_pPtr == other._pPtr && this->_uiAllocatedSize == other._uiAllocatedSize;
		}

		inline bool operator!=( const Blob& other )const
		{
			return !this->operator==( other );
		}

		// is empty blob
		//
		inline operator bool()const
		{
			return !this->empty();
		}
		inline bool operator!()const
		{
			return this->empty();
		}

		inline bool empty()const
		{
			return !this->_pPtr;
		}

		// clean blob
		//
		virtual void clean()
		{
			if( this->_isOwner )
			{
				free( this->_pPtr );
			}
			else
			{
				this->_pPtr = nullptr;
			}
			this->_uiAllocatedSize = 0;
		}

		// allocate buffer
		//
		virtual void allocate( size_t uiSize )
		{
			this->clean();
			this->_uiAllocatedSize = uiSize;

			if( this->_uiAllocatedSize )
			{
				this->_pPtr = malloc( this->_uiAllocatedSize );
				this->_isOwner = true;
			}
		}

		// allocate buffer
		//
		virtual void allocate_zero( size_t uiSize )
		{
			this->clean();
			this->_uiAllocatedSize = uiSize;

			if( this->_uiAllocatedSize )
			{
				this->_pPtr = calloc( 1, this->_uiAllocatedSize );
				this->_isOwner = true;
			}
		}

		virtual void Zeroing()
		{
			if( this->empty() )
			{
				return;
			}
			memset( this->_pPtr, 0, this->_uiAllocatedSize );
		}

		// put data
		//
		virtual void set( void const* const ptr, size_t uiSize )
		{
			if( !this->empty() || this->_uiAllocatedSize < uiSize )
			{
				// create buffer
				this->allocate( uiSize );
			}

			memcpy( this->_pPtr, ptr, uiSize );
		}

		// put data with offset
		//
		virtual void set( void const* const ptr, size_t uiSize, size_t uiOffset )
		{
			if( this->_uiAllocatedSize < ( uiSize + uiOffset ) )
			{
				return;
			}

			char* pData = ( char* )this->_pPtr;
			pData += uiOffset;
			memcpy( pData, ptr, uiSize );
		}

		// getting data
		//
		inline virtual void* get()const
		{
			return this->_pPtr;
		}

		// getting data with offset
		//
		inline virtual void* get( size_t uiOffset )const
		{
			char* pData = ( char* )this->_pPtr;
			if( uiOffset >= this->_uiAllocatedSize )
			{
				// wrong offset
				//
				return nullptr;
			}

			return pData + uiOffset;
		}

		// getting blob size
		//
		inline virtual size_t size()const
		{
			return this->_uiAllocatedSize;
		}

		// helper cast function
		//
		template< typename type >
		inline type* cast_to()const
		{
			return static_cast< type* >( this->get() );
		}

		// helper cast function
		//
		template< typename type >
		inline type* cast_to( size_t uiOffset )const
		{
			return static_cast< type* >( this->get( uiOffset ) );
		}

		// making sub instance of current blob, 
		// memory chunk will not be freed
		//
		Blob Subinstance( size_t uiSize = -1, size_t uiOffset = 0 )
		{
			if( uiSize == -1 )
			{
				uiSize = this->size();
			}

			// making sub instance of current blob, 
			// memory chunk will not be freed
			Blob blob;
			blob._pPtr = this->get( uiOffset );
			blob._uiAllocatedSize = uiSize;
			blob._isOwner = false;

			return blob;
		}
	protected:
		// pointer for data
		//
		void* _pPtr;

		// real size of allocated buffer
		//
		size_t _uiAllocatedSize;

		// is owner of the allocator
		//
		bool _isOwner;
	};

}