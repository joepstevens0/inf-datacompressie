#include "BitStream.h"
#include <string.h>
#include <math.h>
#include <algorithm>

namespace util {
    const uint8_t bitmasks[] = {0, 128, 192, 224, 240, 248, 252, 254};

    BitStreamReader::BitStreamReader( uint8_t *b, int s ) : BitStream( b, s, 0, false )
    {
    }

    BitStreamReader::~BitStreamReader()
    {
        if ( managed )
            clear();
    }

    void BitStreamReader::clear()
    {
        if( buffer != 0 )
            delete [] buffer;

        position = 0;
        size = 0;
    }

    void BitStreamReader::flush()
    {
        if( position % 8 != 0 )
            position += 8 - ( position % 8 );
    }

    uint8_t BitStreamReader::get_bit()
    {
        int bits_taken = position % 8;
        uint8_t value = buffer[position/8];
        value = value & ( 1 << ( 7 - bits_taken ) );
        position++;
        return value != 0;
    }

    uint32_t BitStreamReader::get( int l )
    {
        uint32_t value = 0;

        for ( int i = 0; i < l; i++ ) {
            uint8_t v = get_bit();
            value |= v << ( l - i - 1 );
            //printf("%s %d pos %d i %d v %d value %x\n", __func__, __LINE__, position, i, v, value);
        }

        return value;
    }

    BitStreamWriter::BitStreamWriter( int s ) : BitStream( 0, s, 0, true )
    {
        buffer = new uint8_t[size];
        //printf("Allocated buffer of size: %d\n", s);
    }

    BitStreamWriter::BitStreamWriter( uint8_t *b, int s ) : BitStream( b, s, 0, false )
    {
    }

    BitStreamWriter::~BitStreamWriter()
    {
        if ( managed )
            clear();
    }

    void BitStreamWriter::clear()
    {
        if( buffer != 0 )
            delete [] buffer;

        position = 0;
        size = 0;
    }

    void BitStreamWriter::flush()
    {
        if( position % 8 != 0 ) {
            buffer[position/8] &= bitmasks[position % 8];
            position += 8 - ( position % 8 );
        }
    }

    void BitStreamWriter::put_bit( int8_t value )
    {
        int bits_taken = position % 8;

        if ( value )
            buffer[position/8] |= 1 << ( 7 - bits_taken );
        else
            buffer[position/8] &= ~( 1 << ( 7 - bits_taken ) );

        position++;
    }

    void BitStreamWriter::put( int length, uint32_t value )
    {
        for ( int p = 0; p < length; p++ )
            put_bit( 1 & ( value >> ( length - 1 - p ) ) );
    }

    void write( FILE *f, const BitStreamWriter &b )
    {
        int r;
        int position = b.get_position();
        const uint8_t *buffer = b.get_buffer();
        r = fwrite( buffer, 1, position / 8, f );

        if ( position % 8 != 0 )
            r = fwrite( buffer + position / 8, 1, 1, f );
    }

    void write( std::ofstream &fs, const BitStreamWriter &b )
    {
        int position = b.get_position();
        const uint8_t *buffer = b.get_buffer();
        fs.write( ( const char* )buffer, position / 8 );

        if ( position % 8 != 0 )
            fs.write( ( const char* )( buffer + position / 8 ), 1 );
    }

}

