#ifndef BIT_MACROS_HPP
#define BIT_MACROS_HPP

#define BIT_MASK(pos) (1U << (pos))

// Set bit at pos to 1
#define SET_BIT(x, pos)     \
    do                      \
    {                       \
        x |= BIT_MASK(pos); \
    } while (0)

// Set  bit at pos to 0
#define CLEAR_BIT(x, pos)      \
    do                         \
    {                          \
        x &= (~BIT_MASK(pos)); \
    } while (0)

// Set single bit at pos to val (0 or 1)
#define SET_BIT_VALUE(x, pos, val)                     \
    do                                                 \
    {                                                  \
        x = (x & (~BIT_MASK(pos)) | ((!!val) << (pos))); \
    } while (0)

// Toggle single bit at pos
#define TOGGLE_BIT(x, pos)  \
    do                      \
    {                       \
        x ^= BIT_MASK(pos); \
    } while (0)

// Returns 1 if bit is set, otherwise 0.
#define IS_BIT_SET(x, pos) (!!(x & BIT_MASK(pos)))

// Get low nibble (4 bits) of a byte. 0x34 -> 0x4
#define LONIBBLE(b) ( ((uint8_t)(b)) & 0xf )

// Get hi nibble (4 bits) of a byte. 0x34 -> 0x3
#define HINIBBLE(b) ((uint8_t) ((((uint8_t)(b)) & 0xf0 ) >> 4))

// Get low byte of a word. 0x1234 -> 0x34
#define LOBYTE(w) ((uint8_t)(((uint16_t)(w)) & 0xff))

// Get high byte of a word. 0x1234 -> 0x12
#define HIBYTE(w) ((uint8_t)((((uint16_t)(w)) >> 8) & 0xff))

#endif
