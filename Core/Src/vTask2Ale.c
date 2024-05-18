#include "main.h"

#define Trig_high		GPIOA->BSRR=GPIO_BSRR_BS_0 			// turn on 	PA0 (trig pin)
#define Trig_low		GPIOA->BSRR=GPIO_BSRR_BR_0 			// turn off PA0 (trig pin)

#define us_per_Tick 2.7f	//34% error (2.68us instead of 2us with the oscilloscope)

uint32_t numTicks;

//Speed of sound in cm/us divided by two
const float speed_of_sound = 0.0343 / 2;
float distance;

extern SemaphoreHandle_t printMutex;

volatile void vTask2Ale(void *pvParameters){

    for(;;) {
    	//Connect acting like a sender
    	csp_conn_t *conn = csp_connect(CSP_PRIO_NORM, NODE_ADDRESS_RECEIVER, CSP_PORT, 50, CSP_O_NONE);
    	if(conn == NULL){
    		FF_PRINTF("Connection failed \n\r");
    		continue;
    	}

		//Set TRIG to LOW for few us
		HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
		delayuS(3);

		//HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_SET);
    	Trig_high;
		delayuS(2);
		//HAL_GPIO_WritePin(TRIG_GPIO_Port, TRIG_Pin, GPIO_PIN_RESET);
		Trig_low;

		// Wait for pulse to arrive
		while(!(GPIOA->IDR & GPIO_PIN_1));

		//Start measuring pulse width in us
		numTicks = 0;
		while(GPIOA->IDR & GPIO_PIN_1){
		  numTicks++;
		  delayuS(2);
		}

		//Estimate distance
		distance = (numTicks + 0.0f) * us_per_Tick * speed_of_sound;
		if(xSemaphoreTake(printMutex, (TickType_t)10) == pdTRUE) {
			FF_PRINTF("Distance %f cm\r\n", distance);
			xSemaphoreGive(printMutex);
		}

		//Create CSP packet
		csp_packet_t* packet = csp_buffer_get(sizeof(float));
		if (packet != NULL) {
		            memcpy(packet->data, &distance, sizeof(float));
		            packet->length = sizeof(float);
		            csp_send(conn, packet);
		        }

	    // Close the connection
	    csp_close(conn);

		HAL_Delay(100);
    }

    vTaskDelete(NULL);
}
