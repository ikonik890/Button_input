#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/pwm.h"
#include <map>

#define BTTN1_PIN 2
#define BTTN2_PIN 3
#define BTTN3_PIN 4
#define LED_PIN 20

const uint bounceDelay = 300;
static uint8_t LED_Brightness = 255;
static bool LED_Enabled = true;

static std::map<int, uint32_t> lastButtonFire = {
    {BTTN1_PIN, 0},
    {BTTN2_PIN, 0},
    {BTTN3_PIN, 0}
};

void set_led_brightness(uint gpioPin, uint8_t brightness) {
    uint sliceNum = pwm_gpio_to_slice_num(gpioPin);
    pwm_set_chan_level(sliceNum, PWM_CHAN_A, brightness);
    printf("set LED brightness to  %d\n", brightness);
}

void buttons_callback(uint gpio, uint32_t event_mask) {
    uint32_t currentTime = to_ms_since_boot(get_absolute_time());

    //Restict button presses to once every 300ms to prevent bouncing, check if current time < lastButtonFire to reset timer on overflow
    if(bounceDelay < currentTime - lastButtonFire[gpio] || currentTime < lastButtonFire[gpio]) {
        printf("button %d pressed!\n", gpio);
        lastButtonFire[gpio] = currentTime;

        switch (gpio) {

            //BTTN1 reduces LED brightness
            case BTTN1_PIN:
                if(LED_Brightness)
                    LED_Brightness -= 15;
            break;

            //BTTN2 increases LED brightness
            case BTTN2_PIN:
                if(LED_Brightness < 255)
                    LED_Brightness += 15;
            break;

            //BTTN3 enables and disables the LED
            case BTTN3_PIN:
                LED_Enabled = !LED_Enabled;
            break;
        }

        if(LED_Enabled) {
            set_led_brightness(LED_PIN, LED_Brightness);
        } else {
            set_led_brightness(LED_PIN, 0);
        }
    }
}

void setup() {
    stdio_init_all();
    sleep_ms(1000);

    gpio_init(BTTN1_PIN);
    gpio_init(BTTN2_PIN);
    gpio_init(BTTN3_PIN);

    gpio_set_dir(BTTN1_PIN, GPIO_IN);
    gpio_set_dir(BTTN2_PIN, GPIO_IN);
    gpio_set_dir(BTTN3_PIN, GPIO_IN);

    gpio_pull_up(BTTN1_PIN);
    gpio_pull_up(BTTN2_PIN);
    gpio_pull_up(BTTN3_PIN);

    gpio_set_irq_enabled_with_callback(BTTN1_PIN, GPIO_IRQ_EDGE_FALL, true, buttons_callback);
    gpio_set_irq_enabled(BTTN2_PIN, GPIO_IRQ_EDGE_FALL, true);
    gpio_set_irq_enabled(BTTN3_PIN, GPIO_IRQ_EDGE_FALL, true);

    //Set up PWM for LED
    gpio_set_function(LED_PIN, GPIO_FUNC_PWM);
    uint sliceNum = pwm_gpio_to_slice_num(LED_PIN);
    pwm_set_wrap(sliceNum, 255);
    set_led_brightness(LED_PIN, LED_Brightness);
    pwm_set_enabled(sliceNum, true);
}


int main()
{
    setup();

    while (true) {
        sleep_ms(5000);
        printf("Brightness: %d, Enabled: %d\n", LED_Brightness, LED_Enabled);
    }
}
