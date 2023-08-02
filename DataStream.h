#pragma once

#ifdef _WIN32
#include <winsock.h>
#else
#include <sys/socket.h>
#endif

#include <cstdint>
#include <string>
#if ( defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L ) || __cplusplus >= 201703L
#include <string_view>
#endif

// base class
class BaseSocketClass
{
public:
	std::uint8_t* Allocate( const std::uint32_t size );
	void Deallocate( std::uint8_t* address );

	std::uint32_t FloatToIntBits( float v );
	std::uint64_t DoubleToLongBits( double v );

	float IntBitsToFloat( std::uint32_t v );
	double LongBitsToDouble( std::uint64_t v );
};

// DataInputStream
class DataInputStream : public BaseSocketClass
{
public:
	// constructor
	DataInputStream( SOCKET socket );

	// functions
	bool ReadBoolean();
	std::uint8_t ReadByte();
	// dont need ReadUnsignedByte here, std::uint8_t is unsigned already and you can easily convert anyway
	short ReadShort();
	int ReadUnsignedShort();
	std::uint8_t ReadChar();
	int ReadInt();
	std::uint64_t ReadLong();
	float ReadFloat();
	double ReadDouble();
	std::wstring ReadUTF();
private:
	SOCKET socket;

	// socket receive function
	bool Receive( std::uint8_t* buf, int length );
};

// DataOutputStream
class DataOutputStream : public BaseSocketClass
{
public:
	// constructor
	DataOutputStream( SOCKET socket );

	// functions
	bool WriteBoolean( bool v );
	bool WriteByte( std::uint8_t v );
	bool WriteShort( short v );
	bool WriteChar( std::uint8_t v );
	bool WriteInt( int v );
	bool WriteLong( std::uint64_t v );
	bool WriteFloat( float v );
	bool WriteDouble( double v );
	bool WriteBytes( std::uint8_t* bytes, const std::uint32_t size );
	int WriteUTF(
#if ( defined( _MSVC_LANG ) && _MSVC_LANG >= 201703L ) || __cplusplus >= 201703L
		const std::wstring_view str
#else
		const std::wstring str
#endif
	);
private:
	SOCKET socket;

	// socket send function
	bool Send( std::uint8_t* bytes, const std::uint32_t size );
};