#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#
#include "delay.h"   
#include "gpio.h"       
#include "interrupt.h"   
#include "misc.h"        
#include "soft_timer.h" 
#include "event_queue.h"
#include "gpio.h"
#include "status.h"
#include "digital_input.h"
//Gpio Addresses

static const GpioAddress steering_digital_input[] {
    [STEERING_DIGITAL_INPUT_ID_HORN] = {
    .port= GPIO_PORT_B
    .pin=0
    [STEERING_DIGITAL_INPUT_ID_RADIO_PPT] = {
    .port= GPIO_PORT_B
    .pin=1
    [STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_FWD] = {
    .port= GPIO_PORT_B
    .pin=2
    [STEERING_DIGITAL_INPUT_ID_HIGH_BEAM_BACK] = {
    .port= GPIO_PORT_B
    .pin=3
  },
  

}
int main() {


while(true) {

}
return 0;
}
