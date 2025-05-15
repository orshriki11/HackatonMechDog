#if !defined(led_funstions_hpp)
#define led_funstions_hpp)

// GPIO pin definitions
#define BLUE_LED 1
#define RED_LED 47

void setupLEDs();
void turnOnBlueLED();
void turnOffBlueLED();
void blinkRedLED(int times = 1, int delay_ms = 100);

#endif // led_funstions_hpp)
