#include "xparameters.h"
#include "xil_printf.h"
#include "sleep.h"
#include "string.h"
#include "PmodESP32.h"

#ifdef __MICROBLAZE__
#define HOST_UART_DEVICE_ID XPAR_AXI_UARTLITE_0_BASEADDR
#define HostUart XUartLite
#define HostUart_Config XUartLite_Config
#define HostUart_CfgInitialize XUartLite_CfgInitialize
#define HostUart_LookupConfig XUartLite_LookupConfig
#define HostUart_Recv XUartLite_Recv
#define HostUartConfig_GetBaseAddr(CfgPtr) (CfgPtr->RegBaseAddr)
#include "xuartlite.h"
#include "xil_cache.h"
#else
#define HOST_UART_DEVICE_ID XPAR_PS7_UART_0_DEVICE_ID
#define HostUart XUartPs
#define HostUart_Config XUartPs_Config
#define HostUart_CfgInitialize XUartPs_CfgInitialize
#define HostUart_LookupConfig XUartPs_LookupConfig
#define HostUart_Recv XUartPs_Recv
#define HostUartConfig_GetBaseAddr(CfgPtr) (CfgPtr->BaseAddress)
#include "xuartps.h"
#endif

#define PMODESP32_UART_BASEADDR XPAR_PMODESP32_0_AXI_LITE_UART_BASEADDR
#define PMODESP32_GPIO_BASEADDR XPAR_PMODESP32_0_AXI_LITE_GPIO_BASEADDR

void EnableCaches();
void DisableCaches();
void DemoInitialize();
void DemoRun();
void DemoCleanup();

PmodESP32 myESP32;
HostUart myHostUart;


void DemoInitialize () {
	HostUart_Config *CfgPtr;
	EnableCaches();
	ESP32_Initialize(&myESP32, PMODESP32_UART_BASEADDR, PMODESP32_GPIO_BASEADDR);
	CfgPtr = HostUart_LookupConfig(HOST_UART_DEVICE_ID);
	HostUart_CfgInitialize(&myHostUart, CfgPtr, HostUartConfig_GetBaseAddr(CfgPtr));
}
void setAccessPoint(){
	u8 accessPointAT[]="AT+CWMODE=3\r\n";
	u32 num=strlen((char*)accessPointAT);
	ESP32_SendBuffer(&myESP32,accessPointAT,num);
	usleep(1000);
}
void setAPConf(){
	u8 setAPAT[]="AT+CWSAP=\"myESP32\",\"myESP32pass\",6,4\r\n";
	u32 lengthSetAP=strlen((char*)setAPAT);
	ESP32_SendBuffer(&myESP32,setAPAT,lengthSetAP);
	usleep(1000);
}
void cipStart (){
	u8 cipStartAT[]="AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80\r\n";
	u32 lenghtCipSend = strlen ((char*)cipStartAT);
	ESP32_SendBuffer(&myESP32,cipStartAT,lenghtCipSend);
	usleep(1000);

}

void wifiConnection(){
	u8 connectWifi[]="AT+CWJAP=\"samet\",\"123456789\",\r\n";
	u32 lengthConnectWifi=strlen((char*)connectWifi);
	ESP32_SendBuffer(&myESP32,connectWifi,lengthConnectWifi);
	usleep(1000);
}

void DemoCleanup() {
	DisableCaches();
}


int main() {
	DemoInitialize();
	setAccessPoint();
	setAPConf();
	wifiConnection();
	cipStart();
	u8 recv_buffer=0;
	//DemoRun();
	while(1){
		u8 tempBuff[100];
		u8 tempBuffFinal[100];
		sprintf ((char*)tempBuff,"GET https://api.thingspeak.com/channels/Sicaklik/feeds.json");
		u8 tempBuffLen=strlen((char*)tempBuff);
		sprintf ((char*)tempBuffFinal,"AT+CIPSEND=%d\r\n",(int)tempBuffLen);
		ESP32_SendBuffer(&myESP32,tempBuffFinal,tempBuffLen);
		usleep(1000);

		int tempRecv=ESP32_Recv(&myESP32,&recv_buffer,1);

		u8 humBuff[100];
		u8 humBuffFinal[100];
		sprintf ((char*)humBuff,"GET https://api.thingspeak.com/channels/Nem/feeds.json");
		u8 humBuffLen=strlen((char*)humBuff);
		sprintf ((char*)humBuffFinal,"AT+CIPSEND=%d\r\n",(int)humBuffLen);
		ESP32_SendBuffer(&myESP32,humBuffFinal,humBuffLen);

		int humRecv=ESP32_Recv(&myESP32,&recv_buffer,1);


		xil_printf ("Temperature = %d\n",tempRecv);
		xil_printf ("Humidity= %d\n",humRecv);


	}
	return 0;
}

void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#endif
}

