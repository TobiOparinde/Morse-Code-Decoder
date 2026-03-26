/* Seven-segment display (5161BS) */
// http://www.xlitx.com/datasheet/5161BS.pdf

// (Raspberry Pi Pico pinout: https://datasheets.raspberrypi.com/pico/Pico-R3-A4-Pinout.pdf)

#pragma once

/** Top-center horizontal bar. */
#define SEGMENT_A            7  // Pin 10 (GPIO 07)

/** Top-right vertical bar */
#define SEGMENT_B            6  // Pin 09 (GPIO 06)

/** Bottom-right vertical bar */
#define SEGMENT_C           20  // Pin 26 (GPIO 20)

/** Bottom-center horizontal bar */
#define SEGMENT_D           19  // Pin 25 (GPIO 19)

/** Bottom-left vertical bar */
#define SEGMENT_E           18  // Pin 24 (GPIO 18)

/** Top-left vertical bar */
#define SEGMENT_F            8  // Pin 11 (GPIO 08)

/** Center horizontal bar */
#define SEGMENT_G            9  // Pin 12 (GPIO 09)

/** An array containing the pins for all segments. */
static const unsigned int ALL_SEGMENTS[] = {SEGMENT_A, SEGMENT_B, SEGMENT_C, SEGMENT_D, SEGMENT_E, SEGMENT_F, SEGMENT_G};

#define ALL_SEGMENTS_COUNT  7

uint8_t values[] = {
    0b11111100,
    0b01100000,
    0b11011010,
    0b11110010,
    0b01100110,
    0b10110110,
    0b10111110,
    0b11100000,
    0b11111110,
    0b11110110,
};


/* Initialise the GPIO pins for the seven-segment display. */
void seven_segment_init() {
    for (unsigned int i = 0; i < ALL_SEGMENTS_COUNT; i++) {
        gpio_init(ALL_SEGMENTS[i]);
        gpio_set_dir(ALL_SEGMENTS[i], GPIO_OUT);
    }    
}

/* Turn on/off the seven-segment display. */
void seven_segment_on_off(bool off) {
    for (unsigned int i = 0; i < ALL_SEGMENTS_COUNT; i++) {
        gpio_put(ALL_SEGMENTS[i], off);
    }
}

// turns on specific segments as instructed in parameter
void sevenSegmentOnSpecific(uint8_t segments) {
    for (unsigned int i = 0; i < 8; i++) {
        unsigned int segmentBit = 1 << (7 - i);
        bool illuminateSegment = (segments & segmentBit);
        gpio_put(ALL_SEGMENTS[i], !illuminateSegment);
    }
}

unsigned int seven_segment_show(unsigned int number) {
    if (number > 9) return 1;

    for (unsigned int i = 0; i < 8; i++) {
        // Create a mask to check the current bit.
        //
        // Our first segment (A) starts in the most-significant
        // bit of the bitfield, so we need to 7 - i to map i to
        // the bit for the relevant segment as follows:
        //
        // i = 0 -> 7 - i = 7 - 0 = 7
        // i = 1 -> 7 - i = 7 - 1 = 6
        // ...
        // i = 6 -> 7 - i = 7 - 6 = 1
        // i = 7 -> 7 - i = 7 - 7 =  0
        unsigned int segmentBit = 1 << (7 - i);

        // When the segmentBit is 1 << 7, it produces 0b1000_0000.
        // Where 1 is in the most significant bit and all the other
        // bits are 0.

        // Bitwise-and the segmentBit with the bitfield for the
        // current number. If we get zero, it means that segment
        // should turned off, otherwise the bit is turned on.
        bool illuminateSegment = (segmentBit & values[number]) != 0;

        // Recall, however, that because the display is common-anode,
        // whenever we want to illuminate a segment, we actually need
        // to pull that pin low (i.e., put false).
        gpio_put(
            ALL_SEGMENTS[i],
            !illuminateSegment
        );
    }

    return 0;

}

/**
 * Called once at the start of the program to initialize all of the GPIO
 * pins responsible for the seven-segment display.
 */
void seven_segment_init();

/** Turn off the entire seven-segment display. */
void seven_segment_off();

/** Turn on the entire seven-segment display. */
void seven_segment_on();

/**
 * Displays the specified number on the seven-segment display. (The current function
 * should support all numbers from 0 to 4, you need to extend to display more.)
 * 
 * If the number cannot be displayed (e.g., it's out of range), this
 * method should return a non-zero value.
 * 
 * If the number is valid, the number should be displayed and
 * this method should return 0 to indicate success.
 * 
 * @param number The number to display on the screen.
 * @return Zero if the number has been successfully displayed, otherwise a non-zero
 * value.
 */