### Analog Clock: Product Specification

Time is displayed on a ring of 16 LEDs, in a way similar to an analog wall clock. User may set the time using a UART interface.

- Red light would indicate the position of the Hour hand. similarly, Blue and Green would indicate the presence of Minutes and Seconds hands of the clock.

### Related Diagrams


![Alt text](images/embedded1.png?raw=true "Title") 
![Alt text](images/embedded2.png?raw=true "Title") 

### Communicating with the LED Rings

- The LEDs on the ring are WS2812B LEDs.
- Each LED accepts 24 bits. The first, second and third bytes control the brightness of the green, red and blue LEDs respectively.

A bitstream would be sent to the LEDs using the following protocol:

- Each bit takes 1.25 us to get transmitted
- To transmit bit 0, send high for 0.4 us, and send low for 0.85 us.
- To transmit bit 1, send high for 0.8 us, and send low for 0.45 us.

To reach all the LEDs, we need to send a stream of 16 x 24 bits.
- The first 24 bits are retained by the first LED, and the remaining are passed onto the next LED.
- The next LED does the same. In this way, 384 bits reach all 16 LEDs.
