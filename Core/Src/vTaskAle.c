/*
 * vTask1.c
 *
 *  Created on: Dec 3, 2023
 *      Author: Alessandro
 */

#include "task1_ale.h"
#include "main.h"

extern SemaphoreHandle_t printMutex;

void vTaskAle(void *pvParameters)
{
	//Create RAM disk
	FF_Disk_t * disk_ale;
	static uint8_t buffer_ale[RAMDISK_SECTOR_SIZE * RAMDISK_SECTOR_COUNT] = {0};
	disk_ale = FF_RAMDiskInit(RAMDISK_NAME,//
			buffer_ale, RAMDISK_SECTOR_COUNT,//
			IOMAN_CACHE_SIZE);

	//Mount newly created disk
	FF_Error_t mounting_error;
	mounting_error = FF_Mount(disk_ale, 0);
	FF_GetErrMessage(mounting_error);

	//Create mock directory
	int dir_ok = 0;
	dir_ok = ff_mkdir("/ram/firstSub");

	//Change dir
	int change_ok = 0;
	change_ok = ff_chdir("/ram/firstSub");

	//Create mock file
	int my_file = 0;
	my_file = ff_fopen("./a.txt", "a+");

	size_t written_items = 0;
	char * hello;
	if(my_file != NULL){
		//Write on it
		hello = "Hello, World!\n\r";
		written_items = ff_fwrite(hello, sizeof(char), strlen(hello) / sizeof(char), my_file);
		//Close before re accessing it in the loop
		int close_ok = 0;
		close_ok = ff_fclose(my_file);
	}


	for(;;)
	{
		//HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_12);

		char* read_buffer = pvPortMalloc(50);
		my_file = ff_fopen("./a.txt", "r");
		written_items = ff_fread(read_buffer, sizeof(char),//
				strlen(hello) / sizeof(char), my_file);

		if(xSemaphoreTake(printMutex, (TickType_t)10) == pdTRUE) {
			FF_PRINTF(read_buffer);
		    xSemaphoreGive(printMutex);
		}

		ff_fclose(my_file);

		vPortFree(read_buffer);

		vTaskDelay(pdMS_TO_TICKS(100));

	}
	vTaskDelete(NULL);
}
