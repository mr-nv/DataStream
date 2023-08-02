#include "DataStream.h"

#ifdef _WIN32
#pragma comment( lib, "ws2_32.lib" )
#endif

// dont care
#pragma warning( disable : 4293 4333 )

/** BaseSocketClass */
std::uint8_t* BaseSocketClass::Allocate( const std::uint32_t size )
{
	return new std::uint8_t[ size ];
}

void BaseSocketClass::Deallocate( std::uint8_t* address )
{
	delete[] address;
}

std::uint32_t BaseSocketClass::FloatToIntBits( float v )
{
	std::uint32_t ret;
	memcpy( &ret, &v, sizeof( ret ) );
	return ret;
}

std::uint64_t BaseSocketClass::DoubleToLongBits( double v )
{
	std::uint64_t ret;
	memcpy( &ret, &v, sizeof( ret ) );
	return ret;
}

/** DataOutputStream */
DataOutputStream::DataOutputStream( SOCKET socket )
{
	this->socket = socket;
}

bool DataOutputStream::Send( std::uint8_t* bytes, const std::uint32_t size )
{
	return ::send( this->socket, reinterpret_cast< const char* >( bytes ), size, 0 ) != SOCKET_ERROR;
}

bool DataOutputStream::WriteBoolean( bool v )
{
	auto address = Allocate( 1 );
	address[ 0 ] = v ? 1 : 0;

	bool result = Send( address, 1 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteByte( std::uint8_t v )
{
	auto address = Allocate( 1 );
	address[ 0 ] = v;

	bool result = Send( address, 1 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteShort( short v )
{
	auto address = Allocate( 2 );
	address[ 0 ] = ( v >> 8 ) & 0xFF;
	address[ 1 ] = ( v >> 0 ) & 0xFF;

	bool result = Send( address, 2 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteChar( std::uint8_t v )
{
	auto address = Allocate( 2 );
	address[ 0 ] = ( v >> 8 ) & 0xFF;
	address[ 1 ] = ( v >> 0 ) & 0xFF;

	bool result = Send( address, 2 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteInt( int v )
{
	auto address = Allocate( 4 );
	address[ 0 ] = ( v >> 24 ) & 0xFF;
	address[ 1 ] = ( v >> 16 ) & 0xFF;
	address[ 2 ] = ( v >>  8 ) & 0xFF;
	address[ 3 ] = ( v >>  0 ) & 0xFF;

	bool result = Send( address, 4 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteLong( std::uint64_t v )
{
	auto address = Allocate( 8 );
	address[ 0 ] = ( std::uint8_t )( v >> 56 );
	address[ 1 ] = ( std::uint8_t )( v >> 48 );
	address[ 2 ] = ( std::uint8_t )( v >> 40 );
	address[ 3 ] = ( std::uint8_t )( v >> 32 );
	address[ 4 ] = ( std::uint8_t )( v >> 24 );
	address[ 5 ] = ( std::uint8_t )( v >> 16 );
	address[ 6 ] = ( std::uint8_t )( v >>  8 );
	address[ 7 ] = ( std::uint8_t )( v >>  0 );

	bool result = Send( address, 8 );
	Deallocate( address );
	return result;
}

bool DataOutputStream::WriteFloat( float v )
{
	return WriteInt( FloatToIntBits( v ) );
}

bool DataOutputStream::WriteDouble( double v )
{
	return WriteLong( DoubleToLongBits( v ) );
}

bool DataOutputStream::WriteBytes( std::uint8_t* bytes, const std::uint32_t size )
{
	return Send( bytes, size );
}

int DataOutputStream::WriteUTF(
#if ( defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L ) || __cplusplus >= 201703L
	const std::wstring_view str
#else
	const std::wstring str
#endif
)
{
	size_t strlen = str.length();
	int utflen = 0;
	int c, count = 0;

	for( size_t i = 0; i < strlen; i++ )
	{
		c = str[ i ];
		if( ( c >= 0x0001 ) && ( c <= 0x007F ) )
			utflen++;
		else if( c > 0x07FF )
			utflen += 3;
		else
			utflen += 2;
	}

	if( utflen > 65535 )
		return -1;

	auto address = Allocate( ( utflen * 2 ) + 2 );
	address[ count++ ] = ( std::uint8_t )( ( utflen >> 8 ) & 0xFF );
	address[ count++ ] = ( std::uint8_t )( ( utflen >> 0 ) & 0xFF );

	size_t i = 0;
	for( i = 0; i < strlen; i++ )
	{
		c = str[ i ];
		if( !( ( c >= 0x0001 ) && ( c <= 0x007F ) ) )
			break;

		address[ count++ ] = ( std::uint8_t )c;
	}

	for( ; i < strlen; i++ )
	{
		c = str[ i ];
		if( ( c >= 0x0001 ) && ( c <= 0x007F ) )
			address[ count++ ] = ( std::uint8_t )c;
		else if( c > 0x07FF )
		{
			address[ count++ ] = ( std::uint8_t )( 0xE0 | ( ( c >> 12 ) & 0x0F ) );
			address[ count++ ] = ( std::uint8_t )( 0x80 | ( ( c >> 6 ) & 0x3F ) );
			address[ count++ ] = ( std::uint8_t )( 0x80 | ( ( c >> 0 ) & 0x3F ) );
		}
		else
		{
			address[ count++ ] = ( std::uint8_t )( 0xC0 | ( ( c >> 6 ) & 0x1F ) );
			address[ count++ ] = ( std::uint8_t )( 0x80 | ( ( c >> 0 ) & 0x3F ) );
		}
	}

	Send( address, utflen + 2 );
	Deallocate( address );

	return utflen + 2;
}