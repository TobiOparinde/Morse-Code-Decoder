#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "includes/seven_segment.h"
#include "includes/hash_maps.h"
#include "includes/buzzer.h"
#include "includes/potentiometer.h"
#include "includes/rgb.h"

#define LEFT_BUTTON_PIN	    16	// Pin 21 (GPIO 16)
#define RIGHT_BUTTON_PIN	5	// Pin 7  (GPIO 5)
#define LED_PIN             15	// Pin 20 (GPIO 15)
#define LED_PIN2            14	// Pin 19 (GPIO 14)

// global variables
char morseCode[100] = ""; // string holding the current morseCode input
char letter; // char holding the decoded letter
int limit; // integer holding the time limit set at the start of the program
uint64_t start_time_limit; // holds the current time that the start of a letter is being inputted in ms
uint64_t start_time; // holds the current time that the button is pressed in ms
double hold_duration; // double holding the duration of any button holds
char word[5] = ""; // string holding the word, made up of decoded letters
bool looping = true; // holds the coniditon for whether the entire program is looping or not

int errorFrequency = 100; // frequency for errors
int neutralFrequency = 500; // frequency for inputs
int correctFrequency = 900; // frequency for correct inputs
int dotDuration = 150; // maximum dot input duration
int dashDuration = 470; // maximum dash input duration
int outputDuration = 1000; // duration of regular output
int startOutputDuration = 2000; // duration of initial '8' output at the start of the program

// --------------------------------------------------------------------

// function defenitions
bool checkPause(double pause_duration);
void checkHoldValue(double hold_duration);
void startPause();
void appendToMorse(char* morseCode, char symbol);
void playNote(unsigned int frequency);
void resetTimeLimit();
void setTimeLimit();
void inputChecker();
void getDecision();
bool checkTimeLimit();
void timeLimitReached();
void outputResult(int frequency, int r, int g, int b, int outputDuration, bool off);
void playSong();
void reset_components();

// --------------------------------------------------------------------

// Main program: initialises components and runs main program loop
int main() {
	timer_hw->dbgpause = 0;// enable timers during debugging
	stdio_init_all(); // initialize standard I/O for debugging output

	// initialise the seven segment display.
	seven_segment_init();

	// initialise the left button's GPIO pin.
	gpio_init(LEFT_BUTTON_PIN);
	gpio_set_dir(LEFT_BUTTON_PIN, GPIO_IN); // configure as input
	gpio_pull_down(LEFT_BUTTON_PIN); // enable pull-down resistor

	// initialize the buzzer
	buzzer_init();

	// initialize the RGB LED
	setup_rgb();
	show_rgb(0, 0, 0); // set RGB LED to off (0, 0, 0)

    // initialize the potentiometer
	potentiometer_init();

	//----------------------------------------------------------------------------------------------------------------------------------------

	printf("\nWelcome to the morse code decoder\n\n");
	seven_segment_on_off(false); //turns on the seven segment display (argument is 'off')
	sleep_ms(startOutputDuration); // pause for the startup duration global variable
	seven_segment_on_off(true);

	while (looping){ // main program loop
		setTimeLimit(); // sets time limit
		inputChecker(); // checks for user input (dots and dashes) until a word is created
		getDecision(); // decide whether to continue or exit
	}
}

// sets the time limit for morse code input using the potientometer
void setTimeLimit(){
	printf("Set a time limit for each letter input, then press the button\n\n");
	while (true) { // loops until a value is set and loop is broken by 'break'
		unsigned int value = potentiometer_read(9); // stores the current value the potientometer is set on as an integer
		seven_segment_show(value); // shows the value on the seven segment display

		if (gpio_get(LEFT_BUTTON_PIN)) { // runs when the button is pressed
			limit = value * 1000; // sets the limit as the value on the potientometer in ms
			printf("Limit set %ds\n\n", limit/1000); // displays the limit in seconds

			printf("Starting...\n");
			seven_segment_on_off(true); // turns off the seven segment display (argument is 'off')
			sleep_ms(1000);
			printf("GO\n\n");
			resetTimeLimit(); // sets the time that the start of a letter is being inputted
			break; // breaks out the loop so program moves to next main function
		}

		sleep_ms(10); // debounce
	}
}

// checks for user input (dots and dashes) until a word is created
void inputChecker(){
	while (true) {
			if (strlen(word) == 4) { // checks if the decoded word is 4 letters long
				printf("FINISHED\n\n");
				printf("Word: %s\n", word);
				playSong();
				break; //breaks out the loop so program moves to next main function
			}

			if (gpio_get(LEFT_BUTTON_PIN)) { // runs whenever the button is pressed
				playNote(neutralFrequency); // plays a neutral frequency note on the buzzer when the button is pressed

				start_time = time_us_64();  // get start time of button press in ms

				while (gpio_get(LEFT_BUTTON_PIN)){ // runs whenever the button is held and keeps program flow until released
					if (!checkTimeLimit()){ // checks time limit is not reached while button is held
						if (((time_us_64() - start_time) / 1000) >= 700){ // checks button is not held for more than 700ms
							printf("ERROR: Button held for more than 700ms\n");
							outputResult(errorFrequency, 255, 0, 0, outputDuration, false); //Passes frequency, r g b values, buzzing duration and whether display is off
							resetTimeLimit(); //resets the start time of the time limit (not the limit itself)
							morseCode[0] = '\0'; //resets the current morse code input
							break; //breaks out the loop so next word can be processed
						}
					} else { //if time limit is reached while button is held
						timeLimitReached(); // run function that outputs accordingly when time limit is reached
						break; // breaks out the loop so next word can be processed
					}
				} 

				hold_duration = (time_us_64() - start_time) / 1000; //calculates the duration of the hold and stores it
				buzzer_disable(); // disables the buzzer as button is no longer held

				if (hold_duration < 700){ //checks that hold duration is not the error type
					checkHoldValue(hold_duration); // checks if the value is a dot or a dash and appends it to morse code variable
					startPause(); // starts the pause fuction
				}
			}

			if (checkTimeLimit()) { // checks if time limit is reached even if button is not held
				timeLimitReached();
			}

			sleep_ms(10); // debounce
		}
	}

bool checkTimeLimit(){ //Checks if the time limit has been reached, false for reached, true for not reached
	return ((time_us_64() - start_time_limit) / 1000) > limit; // calculates time from the start of the letter input and compares 
}

void timeLimitReached() {  // prints error if time limit reached
	printf("ERROR: Time limit reached\n");
	outputResult(errorFrequency, 0, 0, 0, outputDuration, false); //Passes frequency, r g b values, buzzing duration and whether display is off
	resetTimeLimit(); // resets time limit
	morseCode[0] = '\0'; // resets morse code string
}

void getDecision(){ // Gets Decision whether to continue or terminate the program
	printf("Do you want to continue or exit? Left button for continue, right to exit\n\n");
	while (true){
		if (gpio_get(LEFT_BUTTON_PIN)){
			outputResult(correctFrequency, 0, 255, 0, 1000, true); //Passes frequency, r g b values, buzzing duration and whether display is off (success signal) 

			morseCode[0] = '\0';
			word[0] = '\0'; // resets created word
			reset_components();
			break;
		} else if (gpio_get(RIGHT_BUTTON_PIN)){
			outputResult(errorFrequency, 255, 0, 0, outputDuration, false);
			printf("Goodbye\n");

			looping = false; //main program loop broken so it terminates
			reset_components(); 
			break;
		}
	}
}

void reset_components() { // Reset all components to default state
    // Reset the buzzer
    buzzer_disable();

    // Reset RGB LED
    show_rgb(0, 0, 0);

    seven_segment_on_off(true); // Turn off the display
}


// Function to play a song when a word is successfully decoded
void playSong() {
	unsigned int speedUp[] = { C, E, A4}; // Fast sequence tones
	unsigned int melody[] = { C, E, A4, C, E, A4, C, E, A4, C, E, A4, C, E, A4, C, E, A4, C, E, A4, C, E, A4, B3, E, A4, B3, E, A4, B3, E, A4, B3, E, G4, B3, E, G4, B3, E, G4, B3, E, G4, B3, E, G4}; // note sequence
	unsigned int melodyLength = sizeof(melody)/sizeof(melody[0]); // calculates size of array
	unsigned int speedUpLength = sizeof(speedUp)/sizeof(speedUp[0]); // calculates size of array

	int interval = 250; // sets interval value
	// play speed-up sequence (loops until interval is fast enough)
	while (interval > 110) { 
		for (unsigned int i = 0; i < speedUpLength; i++) {
			buzzer_enable(speedUp[i]);
			sleep_ms(interval);
			buzzer_disable();
			interval -= 5; // speeds up interval
		}
	}

	    // Play full melody twice
	    for (int repeat = 0; repeat < 2; repeat++) {
	        for (unsigned int i = 0; i < melodyLength; i++) {
	            buzzer_enable(melody[i]);
	            sleep_ms(interval);
	            buzzer_disable();
	        }
    	}

	buzzer_disable(); // turns off buzzer
}

void resetTimeLimit(){ // resets time limit to defualt state
	start_time_limit = time_us_64();  // Get start time in microseconds
}

// Handles Pause between inputs
void startPause(){
	uint64_t start_time = time_us_64();  // Get start time in microseconds

	//waits for pause after pressed
	while (!gpio_get(LEFT_BUTTON_PIN)){
		if ((!checkTimeLimit())) { //checks time limit
			if (checkPause((time_us_64() - start_time) / 1000)){ //check if pause duration is valid
				break;
			}
		} else {
			timeLimitReached(); //Handles if time limit reached
			break;
		}
	}
}

// Appends a symbol to Morse code array
void appendToMorse(char* morseCode, char symbol) {
    int len = strlen(morseCode); //gets current length
    morseCode[len] = symbol;   // Append the dot or dash
    morseCode[len + 1] = '\0'; // Ensure the string is null-terminated
}

// checks whether the input is a dot or a dash and appends it to the morseCode string
void checkHoldValue(double hold_duration){
    if (hold_duration < 250.00) {
		appendToMorse(morseCode, '.');
	} else if (hold_duration < 700.00) {
		appendToMorse(morseCode, '-');
	}
}

// plays a passed on frequency on the buzzer
void playNote(unsigned int frequency) {
	buzzer_enable(frequency);
}

// checks whether the pause duration between each button input should be considered as an inter letter gap or not
bool checkPause(double pause_duration){
	if (pause_duration > 400.00) { // checks whether the pause duration is bigger than 400 (inter letter gap)
		letter = decodeMorseToPrint(morseCode); // decodes the morseCode string and stores it
		if (letter != '?' && letter != '!') { // checks that the decoded character is not a empty morsecode or incorrect morsecode result
			printf("%c\n", letter);

			// appends the decoded character to the word string
			int len = strlen(word); 
    		word[len] = letter;
    		word[len + 1] = '\0';

			outputResult(correctFrequency, 0, 255, 0, 500, true); //Passes frequency, r g b values, buzzing duration and whether display is off
			sevenSegmentOnSpecific(decodeMorseToDisplay(morseCode)); // shows the decoded letter on the seven segment display
		} else if (letter == '?') {
			printf("ERROR: Morse code invalid\n");
			outputResult(errorFrequency, 255, 0, 0, outputDuration, false); //Passes frequency, r g b values, buzzing duration and whether display is off
		}
		resetTimeLimit(); // resets the start time of the time limit (not the limit itself)
		morseCode[0] = '\0'; //resets the current morse code input
		return true; // returns true to show that the gap was big enough to decode
	}
	return false; // returns false to show that the gap was NOT big enough to decode
}

// outputs any selected component using parameters
void outputResult(int frequency, int r, int g, int b, int sleepValue, bool off) {
	seven_segment_on_off(off);
	playNote(frequency);
	show_rgb(r, g, b);
	sleep_ms(sleepValue);
	reset_components(); // resets components back to previous state
}

