#include PLATFORM_HEADER
#include BOARD_HEADER
#include "hal/micro/micro-common.h"
#include "hal/micro/cortexm3/micro-common.h"

#include "dev/button-sensor.h"
#include "dev/leds.h"

void halBoardInit(void)
{
  
  return;
}

static uint8_t sensors_status;

#define BUTTON_STATUS_ACTIVE 	(1 << 0)

void halBoardPowerDown(void)
{
	/* Set everything to input value except LEDs */
	  GPIO_PACFGL = (GPIOCFG_IN              <<PA0_CFG_BIT)|
	                (GPIOCFG_IN              <<PA1_CFG_BIT)|
	                (GPIOCFG_IN              <<PA2_CFG_BIT)|
	                (GPIOCFG_IN              <<PA3_CFG_BIT);
	  GPIO_PACFGH = (GPIOCFG_IN              <<PA4_CFG_BIT)|  /* PTI EN */
	                (GPIOCFG_IN              <<PA5_CFG_BIT)|  /* PTI_DATA */
	                (GPIOCFG_IN              <<PA6_CFG_BIT)|
	                (GPIOCFG_IN              <<PA7_CFG_BIT);
	  GPIO_PBCFGL = (GPIOCFG_IN              <<PB0_CFG_BIT)|
	                (GPIOCFG_IN              <<PB1_CFG_BIT)|  /* Uart TX */
	                (GPIOCFG_IN              <<PB2_CFG_BIT)|  /* Uart RX */
	                (GPIOCFG_IN              <<PB3_CFG_BIT);
	  GPIO_PBCFGH = (GPIOCFG_IN              <<PB4_CFG_BIT)|
	                (GPIOCFG_IN              <<PB5_CFG_BIT)|
	                (GPIOCFG_IN              <<PB6_CFG_BIT)|
	                (GPIOCFG_IN              <<PB7_CFG_BIT);
	  GPIO_PCCFGL = (GPIOCFG_IN              <<PC0_CFG_BIT)|
	                (GPIOCFG_IN              <<PC1_CFG_BIT)|
	                (GPIOCFG_IN              <<PC2_CFG_BIT)|
	                (GPIOCFG_IN              <<PC3_CFG_BIT);
	  GPIO_PCCFGH = (GPIOCFG_IN              <<PC4_CFG_BIT)|
	                (GPIOCFG_IN              <<PC5_CFG_BIT)|
	                (GPIOCFG_IN              <<PC6_CFG_BIT)|  /* OSC32K */
	                (GPIOCFG_IN              <<PC7_CFG_BIT);  /* OSC32K */

	  leds_init();

}

/* Remember state of sensors (if active or not), in order to
 * resume their original state after calling powerUpSensors().
 * Useful when entering in sleep mode, since all system
 * peripherals have to be reinitialized.  */

void sensorsPowerDown(){

	sensors_status = 0;

	if(button_sensor.status(SENSORS_READY)){
		sensors_status |= BUTTON_STATUS_ACTIVE;
	}
}

/**/
void sensorsPowerUp(){

	button_sensor.configure(SENSORS_HW_INIT, 0);

	if(sensors_status & BUTTON_STATUS_ACTIVE){
		SENSORS_ACTIVATE(button_sensor);
	}
}

void halBoardPowerUp(void)
{
  /* Set everything to input value */
  GPIO_PACFGL = (GPIOCFG_IN            <<PA0_CFG_BIT)|
                (GPIOCFG_IN            <<PA1_CFG_BIT)|
                (GPIOCFG_IN            <<PA2_CFG_BIT)|
                (GPIOCFG_IN            <<PA3_CFG_BIT);
  GPIO_PACFGH = (GPIOCFG_IN            <<PA4_CFG_BIT)|  /* PTI EN */
                (GPIOCFG_IN            <<PA5_CFG_BIT)|  /* PTI_DATA */
                (GPIOCFG_IN            <<PA6_CFG_BIT)|
                (GPIOCFG_IN            <<PA7_CFG_BIT);
  GPIO_PBCFGL = (GPIOCFG_IN            <<PB0_CFG_BIT)|
                (GPIOCFG_OUT_ALT       <<PB1_CFG_BIT)|  /* Uart TX */
                (GPIOCFG_IN            <<PB2_CFG_BIT)|  /* Uart RX */
                (GPIOCFG_IN            <<PB3_CFG_BIT);
  GPIO_PBCFGH = (GPIOCFG_IN            <<PB4_CFG_BIT)|
                (GPIOCFG_IN            <<PB5_CFG_BIT)|
                (GPIOCFG_IN            <<PB6_CFG_BIT)|
                (GPIOCFG_IN            <<PB7_CFG_BIT);
  GPIO_PCCFGL = (GPIOCFG_IN            <<PC0_CFG_BIT)|
                (GPIOCFG_IN            <<PC1_CFG_BIT)|
                (GPIOCFG_IN            <<PC2_CFG_BIT)|
                (GPIOCFG_IN            <<PC3_CFG_BIT);
  GPIO_PCCFGH = (GPIOCFG_IN            <<PC4_CFG_BIT)|
                (GPIOCFG_IN            <<PC5_CFG_BIT)|
                (GPIOCFG_IN            <<PC6_CFG_BIT)|  /* OSC32K */
                (GPIOCFG_IN            <<PC7_CFG_BIT);  /* OSC32K */

}


