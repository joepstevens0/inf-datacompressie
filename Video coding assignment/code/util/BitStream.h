#ifndef UTIL_BITSTREAM_H
#define UTIL_BITSTREAM_H

#include <stdint.h>
#include <cstdio>
#include <fstream>

namespace util {

    class BitStream {
        public:
            BitStream( uint8_t *b = 0, int s = 0, int p = 0, bool m = false ) : buffer( b ), size( s ), position( p ), managed( m ) {}
            uint8_t* get_buffer() {
                return buffer;
            }
            const uint8_t* get_buffer() const {
                return buffer;
            }
            int get_size() const {
                return size;
            }
            void set_managed( bool m ) {
                managed = m;
            }
            void set_position( int p ) {
                position = p;
            }
            int get_position() const {
                return position;
            }
            void reset() {
                set_position( 0 );
            }

        protected:
            uint8_t *buffer;
            int size; // Size in bytes
            int position; // Position in bits
            bool managed;
    };

    /**
     * Class which eases reading bitwise from a buffer.
     */
    class BitStreamReader : public BitStream {
        public:
            /**
             * Create a bitstreamreader which reads from the provided buffer.
             *
             * @param [in] buffer The buffer from which bits will be read.
             * @param [in] size The size (expressed in bytes) of the buffer from which bits will be read.
             */
            BitStreamReader( uint8_t *buffer, int size );

            ~BitStreamReader();

            /**
             * Deallocate the buffer and reset all fields
             */
            void clear();

            /**
             * Read one bit from the bitstream.
             *
             * @return The value of the bit.
             */
            uint8_t get_bit();

            /**
             * Get l bits from the bitstream
             *
             * @param [in] l number of bits to read
             * @return The value of the bits read
             *
             * buffer: 0101 1100, position==0
             * get(4) returns value 5, position==4
             */
            uint32_t get( int l );

            /**
             * Move the bitwise position pointer to the next byte boundary
             */
            void flush();

    };

    /**
     * Class which eases writing bitwise into a buffer.
     */
    class BitStreamWriter : public BitStream {
        public:
            /**
             * Create a bitstreamwriter which writes into the provided buffer.
             *
             * @param [in] buffer The buffer into which bits will be written.
             * @param [in] size The size (expressed in bytes) of the buffer into which bits will be written.
             */
            BitStreamWriter( uint8_t *buffer, int size );

            /**
             * Create a bitstreamwriter and allocate a buffer for it.
             *
             * @param [in] size The size (expressed in bytes) of the buffer into which bits will be written.
             */
            BitStreamWriter( int s );

            ~BitStreamWriter();

            void clear();

            /**
             * Write one bit into the bitstream.
             * @param [in] value The value to put into the bitstream.
             */
            void put_bit( int8_t value );

            /**
             * Put 'length' bits with value 'value' into the bitstream
             *
             * @param [in] length Number of bits to use for storing the value
             * @param [in] value The value to store
             *
             * buffer: xxxx xxxx, position==0
             * put(4, 5)
             * buffer: 1010 xxxx, position==4
             */
            void put( int length, uint32_t value );

            /**
             * Byte-align: Move the bitwise position pointer to the next byte boundary
             */
            void flush();

    };

    /**
     * Write the contents of the bitstream to the specified file
     */
    void write( FILE *f, const BitStreamWriter &b );
    void write( std::ofstream &fs, const BitStreamWriter &b );

}
#endif /* UTIL_BITSTREAM_H */

