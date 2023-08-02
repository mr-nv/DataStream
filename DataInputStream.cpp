#include "DataStream.h"

#ifdef _WIN32
#pragma comment( lib, "ws2_32.lib" )
#endif

// dont care
#pragma warning( disable : 4293 4333 )

/** BaseSocketClass */
float BaseSocketClass::IntBitsToFloat( std::uint32_t v )
{
	return *( float* )( &v );
}

double BaseSocketClass::LongBitsToDouble( std::uint64_t v )
{
	return *( double* )( &v );
}

/** DataInputStream */
DataInputStream::DataInputStream( SOCKET socket )
{
	this->socket = socket;
}

bool DataInputStream::Receive( std::uint8_t* buf, int length )
{
	return recv( this->socket, reinterpret_cast< char* >( buf ), length, 0 ) == length;
}

bool DataInputStream::ReadBoolean()
{
	auto address = Allocate( 1 );
	Receive( address, 1 );

	bool result = address[ 0 ] != 0;
	Deallocate( address );
	return result;
}

std::uint8_t DataInputStream::ReadByte()
{
	auto address = Allocate( 1 );
	Receive( address, 1 );

	std::uint8_t result = address[ 0 ];
	Deallocate( address );
	return result;
}

short DataInputStream::ReadShort()
{
	auto address = Allocate( 2 );
	Receive( address, 2 );

	auto ch1 = address[ 0 ];
	auto ch2 = address[ 1 ];

	Deallocate( address );

	if( ( ch1 | ch2 ) < 0 )
		return -1;

	return ( short )( ( ch1 << 8 ) + ( ch2 << 0 ) );
}

int DataInputStream::ReadUnsignedShort()
{
	auto address = Allocate( 2 );
	Receive( address, 2 );

	auto ch1 = address[ 0 ];
	auto ch2 = address[ 1 ];

	Deallocate( address );

	if( ( ch1 | ch2 ) < 0 )
		return -1;

	return ( ( ch1 << 8 ) + ( ch2 << 0 ) );
}

std::uint8_t DataInputStream::ReadChar()
{
	auto address = Allocate( 2 );
	Receive( address, 2 );

	auto ch1 = address[ 0 ];
	auto ch2 = address[ 1 ];

	Deallocate( address );

	if( ( ch1 | ch2 ) < 0 )
		return 0;

	return ( std::uint8_t )( ( ch1 << 8 ) + ( ch2 << 0 ) );
}

int DataInputStream::ReadInt()
{
	auto address = Allocate( 4 );
	Receive( address, 4 );

	auto ch1 = address[ 0 ];
	auto ch2 = address[ 1 ];
	auto ch3 = address[ 2 ];
	auto ch4 = address[ 3 ];

	Deallocate( address );

	if( ( ch1 | ch2 | ch3 | ch4 ) < 0 )
		return 0;

	return ( int )( ( ch1 << 24 ) + ( ch2 << 16 ) + ( ch3 << 8 ) + ( ch4 << 0 ) );
}

std::uint64_t DataInputStream::ReadLong()
{
	auto address = Allocate( 8 );
	Receive( address, 8 );

	std::uint64_t value =
		( ( ( std::uint64_t )address[ 0 ] << 56 ) +
		  ( ( std::uint64_t )( address[ 1 ] & 0xFF ) << 48 ) +
		  ( ( std::uint64_t )( address[ 2 ] & 0xFF ) << 40 ) +
		  ( ( std::uint64_t )( address[ 3 ] & 0xFF ) << 32 ) +
		  ( ( std::uint64_t )( address[ 4 ] & 0xFF ) << 24 ) +
		  ( ( std::uint64_t )( address[ 5 ] & 0xFF ) << 16 ) +
		  ( ( std::uint64_t )( address[ 6 ] & 0xFF ) << 8 ) +
		  ( ( std::uint64_t )( address[ 7 ] & 0xFF ) << 0 ) );

	Deallocate( address );

	return value;
}

float DataInputStream::ReadFloat()
{
	return IntBitsToFloat( ReadInt() );
}

double DataInputStream::ReadDouble()
{
	return LongBitsToDouble( ReadLong() );
}

std::wstring DataInputStream::ReadUTF()
{
	int utflen = ReadUnsignedShort();

	std::uint8_t* bytearr = new std::uint8_t[ utflen * 2 ];
	wchar_t* chararr = new wchar_t[ utflen * 2 ];

	int c, char2, char3;
	int count = 0;
	int chararr_count = 0;

	Receive( bytearr, utflen );

	while( count < utflen )
	{
		c = ( int )bytearr[ count ] & 0xFF;
		if( c > 127 )
			break;

		count++;
		chararr[ chararr_count++ ] = ( wchar_t )c;
	}

	while( count < utflen )
	{
		c = ( int )bytearr[ count ] & 0xff;
		switch( c >> 4 )
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				count++;
				chararr[ chararr_count++ ] = ( wchar_t )c;
				break;
			case 12:
			case 13:
				count += 2;

				if( count > utflen ) // "malformed input: partial character at end"
				{
					delete[] bytearr;
					delete[] chararr;
					return L"";
				}

				char2 = ( int )bytearr[ count - 1 ];
				if( ( char2 & 0xC0 ) != 0x80 ) // "malformed input around byte " + count
				{
					delete[] bytearr;
					delete[] chararr;
					return L"";
				}

				chararr[ chararr_count++ ] = ( wchar_t )( ( ( c & 0x1F ) << 6 ) | ( char2 & 0x3F ) );
				break;
			case 14:
				count += 3;

				if( count > utflen ) // "malformed input: partial character at end"
				{
					delete[] bytearr;
					delete[] chararr;
					return L"";
				}

				char2 = ( int )bytearr[ count - 2 ];
				char3 = ( int )bytearr[ count - 1 ];
				if( ( ( char2 & 0xC0 ) != 0x80 ) || ( ( char3 & 0xC0 ) != 0x80 ) )
				{
					delete[] bytearr;
					delete[] chararr;
					return L"";
				}

				chararr[ chararr_count++ ] = ( wchar_t )( ( ( c & 0x0F ) << 12 ) |
													   ( ( char2 & 0x3F ) << 6 ) |
													   ( ( char3 & 0x3F ) << 0 ) );
				break;
			default:
				// "malformed input around byte " + count
				delete[] bytearr;
				delete[] chararr;
				return L"";
		}
	}

	std::wstring ret( chararr, chararr_count );
	delete[] bytearr;
	delete[] chararr;
	return ret;
}