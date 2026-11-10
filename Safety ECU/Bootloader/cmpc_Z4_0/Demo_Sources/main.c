/**
 * @file main.c
 * @brief Main routine
 *
 * @section License
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 *
 * Copyright (C) 2010-2025 Oryx Embedded SARL. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 * @author Oryx Embedded SARL (www.oryx-embedded.com)
 * @version 2.5.4
 **/

//Dependencies
#include <stdlib.h>
#include "mpc5748g.h"
#include "system_mpc5748g.h"
#include "devkit_mpc5748g.h"
#include "core/net.h"
#include "drivers/mac/mpc5748_eth1_driver.h"
#include "drivers/phy/ksz8081_driver.h"
#include "dhcp/dhcp_client.h"
#include "ipv6/slaac.h"
#include "http/http_server.h"
#include "http/mime.h"
#include "path.h"
#include "date_time.h"
#include "resource_manager.h"
#include "debug.h"
#include "DoIP.h"
#include "Bootloader.h"
#include "flash_c55_driver.h"




//Ethernet interface configuration
#define APP_IF_NAME "eth0"
#define APP_HOST_NAME "http-server-demo"
#define APP_MAC_ADDR "00-AB-CD-EF-57-48"

#define APP_USE_DHCP_CLIENT DISABLED
#define APP_IPV4_HOST_ADDR "192.168.2.20"
#define APP_IPV4_SUBNET_MASK "255.255.255.0"
#define APP_IPV4_DEFAULT_GATEWAY "192.168.2.254"
#define APP_IPV4_PRIMARY_DNS "8.8.8.8"
#define APP_IPV4_SECONDARY_DNS "8.8.4.4"

#define APP_USE_SLAAC DISABLED
#define APP_IPV6_LINK_LOCAL_ADDR "fe80::5748"
#define APP_IPV6_PREFIX "2001:db8::"
#define APP_IPV6_PREFIX_LENGTH 64
#define APP_IPV6_GLOBAL_ADDR "2001:db8::5748"
#define APP_IPV6_ROUTER "fe80::1"
#define APP_IPV6_PRIMARY_DNS "2001:4860:4860::8888"
#define APP_IPV6_SECONDARY_DNS "2001:4860:4860::8844"

#define APP_TCP_SERVER_PORT 7
#define APP_TCP_BUFFER_SIZE 256


#define UDS_DIAGNOSTIC_SESSION_CONTROL     0x10
#define UDS_READ_DATA_BY_IDENTIFIER        0x22
#define UDS_READ_MEMORY_BY_ADDRESS         0x23
#define UDS_READ_SCALING_DATA_BY_ID        0x24
#define UDS_READ_DATA_BY_PERIODIC_ID       0x2A
#define UDS_DYNAMICALLY_DEFINE_DATA_ID     0x2C
#define UDS_WRITE_DATA_BY_IDENTIFIER       0x2E
#define UDS_WRITE_MEMORY_BY_ADDRESS        0x3D
#define UDS_CLEAR_DIAGNOSTIC_INFORMATION   0x14
#define UDS_READ_DTC                       0x19
#define UDS_TESTER_PRESENT                 0x3E

//Application configuration
#define APP_HTTP_MAX_CONNECTIONS 8

//Global variables
DhcpClientSettings dhcpClientSettings;
DhcpClientContext dhcpClientContext;
SlaacSettings slaacSettings;
SlaacContext slaacContext;
HttpServerSettings httpServerSettings;
HttpServerContext httpServerContext;
HttpConnection httpConnections[APP_HTTP_MAX_CONNECTIONS];



/*
 * This binds the variable perfectly into the section we created in the linker script.
 * 'volatile' prevents the compiler from optimizing it out if it doesn't see direct usage.
 */


typedef enum
{
   DIAG_STATE_IDLE = 0,
   DIAG_STATE_MAIN_MENU = 1,
   DIAG_STATE_ENGINE_MENU = 2,
   DIAG_STATE_TRANSMISSION_MENU = 3,
   DIAG_STATE_RUNNING_TEST = 4
} DiagnosticState;

typedef struct
{
   DiagnosticState state;
   Socket *socket;
   char_t buffer[APP_TCP_BUFFER_SIZE];
   size_t bufferPos;
} DiagnosticSession;

//Forward declaration of functions
error_t httpServerCgiCallback(HttpConnection *connection,
   const char_t *param);

error_t httpServerRequestCallback(HttpConnection *connection,
   const char_t *uri);

error_t httpServerUriNotFoundCallback(HttpConnection *connection,
   const char_t *uri);


/**
 * @brief System initialization
 **/

void systemInit(void)
{
   //Set S160 clock divider
   MC_CGM->SC_DC0 = MC_CGM_SC_DC0_DE_MASK | MC_CGM_SC_DC0_DIV(0);
   //Set S80 clock divider
   MC_CGM->SC_DC1 = MC_CGM_SC_DC1_DE_MASK | MC_CGM_SC_DC1_DIV(1);
   //Set S40 clock divider
   MC_CGM->SC_DC2 = MC_CGM_SC_DC2_DE_MASK | MC_CGM_SC_DC2_DIV(3);
   //Set FS80 clock divider
   MC_CGM->SC_DC5 = MC_CGM_SC_DC5_DE_MASK | MC_CGM_SC_DC5_DIV(1);

   //Enable all modes
   MC_ME->ME |= MC_ME_ME_STANDBY0_MASK | MC_ME_ME_STOP0_MASK |
      MC_ME_ME_RUN3_MASK | MC_ME_ME_RUN2_MASK | MC_ME_ME_RUN1_MASK;

   //Set run modes
   MC_ME->RUN_PC[0] = MC_ME_RUN_PC_RUN3_MASK | MC_ME_RUN_PC_RUN2_MASK |
      MC_ME_RUN_PC_RUN1_MASK | MC_ME_RUN_PC_RUN0_MASK |
      MC_ME_RUN_PC_DRUN_MASK | MC_ME_RUN_PC_SAFE_MASK;

   //Connect FXOSC as source clock
   MC_CGM->AC5_SC = MC_CGM_AC5_SC_SELCTL_MASK;

   //Configure PLL divide register
   PLLDIG->PLLDV = PLLDIG_PLLDV_RFDPHI1(6) | PLLDIG_PLLDV_RFDPHI(1) |
      PLLDIG_PLLDV_PREDIV(2) | PLLDIG_PLLDV_MFD(32);

   //Set PLL calibration register
   PLLDIG->PLLCAL3 = PLLDIG_PLLCAL3_MFDEN(9999);

   //Enable sigma delta modulation
   PLLDIG->PLLFD |= PLLDIG_PLLFD_SMDEN_MASK;

   //Configures system behavior during DRUN mode
   MC_ME->DRUN_MC = MC_ME_DRUN_MC_MVRON_MASK | MC_ME_DRUN_MC_FLAON(3) |
      MC_ME_DRUN_MC_SXOSCON_MASK | MC_ME_DRUN_MC_PLLON_MASK |
      MC_ME_DRUN_MC_FXOSCON_MASK | MC_ME_DRUN_MC_SYSCLK(2);

   //Switch to DRUN mode
   MC_ME->MCTL = MC_ME_MCTL_TARGET_MODE(3) | MC_ME_MCTL_KEY(0x5AF0);
   MC_ME->MCTL = MC_ME_MCTL_TARGET_MODE(3) | MC_ME_MCTL_KEY(0xA50F);

   //Wait for the mode transition to complete
   while((MC_ME->GS & MC_ME_GS_S_MTRANS_MASK) != 0)
   {
   }
}


/**
 * @brief I/O initialization
 **/

void ioInit(void)
{
   //Configure LEDs
   SIUL2->MSCR[USR_LED1] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED2] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED3] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED4] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED5] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED6] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED7] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;
   SIUL2->MSCR[USR_LED8] = SIUL2_MSCR_OBE_MASK | SIUL2_MSCR_SMC_MASK;

   //Clear LEDs
   SIUL2->GPDO[USR_LED1] = 1;
   SIUL2->GPDO[USR_LED2] = 1;
   SIUL2->GPDO[USR_LED3] = 1;
   SIUL2->GPDO[USR_LED4] = 1;
   SIUL2->GPDO[USR_LED5] = 1;
   SIUL2->GPDO[USR_LED6] = 1;
   SIUL2->GPDO[USR_LED7] = 1;
   SIUL2->GPDO[USR_LED8] = 1;

   //Configure push buttons
   SIUL2->MSCR[PB_SW1] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
   SIUL2->MSCR[PB_SW2] = SIUL2_MSCR_SMC_MASK | SIUL2_MSCR_IBE_MASK;
}



void ledTask(void *param)
{
   //Endless loop
   while(1)
   {
      SIUL2->GPDO[USR_LED1] = 0;
      osDelayTask(100);
      SIUL2->GPDO[USR_LED1] = 1;
      osDelayTask(900);
   }
}

/**
 * @brief Main entry point
 * @return Unused value
 **/



void Core_Boot(void)

{
	/* Enable e200z4b and e200z2 cores in RUN0-RUN3, DRUN and SAFE modes */
	uint32_t mctl = MC_ME->MCTL;

	MC_ME->CCTL2 = 0x00FEU;    /* e200z4b is active */

	/* Set start address for e200z4b and e200z2 cores */

	MC_ME->CADDR2 = 0x01040000 | 0x1U; /* e200z4b boot address + RMC bit */

	/* Mode change - re-enter the DRUN mode to start cores */


	MC_ME->MCTL = 0x30005AF0;         /* Mode & Key */

	MC_ME->MCTL = 0x3000A50F;        /* Mode & Key inverted */

	//	MC_ME->MCTL = (mctl & 0xffff0000ul) | KEY_VALUE1;
	//	MC_ME->MCTL =  mctl;

	while((MC_ME->GS & (1 << 27)) == 1);   /* Wait for mode entry complete */

	while (((MC_ME->GS >> 28) & 0xF) != 0x3); /* Check DRUN mode entered */

	//

}//Core_Boot

void DisableResetEscalation (void){
	/*Reset Escalation Registers*/
	uint8_t * const FRETRegister = (uint8_t *)0xFFFA8604;
	uint8_t * const DRETRegister = (uint8_t *)0xFFFA8608;

	/*Clearing those registers; see datasheet p2809, 2810*/
	*FRETRegister = 0U;
	*DRETRegister = 0U;
}



BL_Functions a_BLHandlersConfig = {.BL_TransferDataHandler = Bootloader_Program,
		.BL_Check_Memory = Bootloader_CheckMemory,
		.BL_Erase_Memory = Bootloader_Erase_Memory,
		.BL_Finalize_Programming = Bootloader_Finalize_Programming};


int_t main(void)
{
   error_t error;
   OsTaskId taskId;
   OsTaskParameters taskParams;
   NetInterface *interface;
   MacAddr macAddr;
#if (APP_USE_DHCP_CLIENT == DISABLED)
   Ipv4Addr ipv4Addr;
#endif
#if (APP_USE_SLAAC == DISABLED)
   Ipv6Addr ipv6Addr;
#endif

   //System initialization
   systemInit();
   //Update system core clock
   SystemCoreClockUpdate();

   //Initialize kernel
   osInitKernel();
   //Configure debug UART
   debugInit(19200);

   //Start-up message
   TRACE_INFO("\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("*** CycloneTCP HTTP Server Demo ***\r\n");
   TRACE_INFO("***********************************\r\n");
   TRACE_INFO("Copyright: 2010-2025 Oryx Embedded SARL\r\n");
   TRACE_INFO("Compiled: %s %s\r\n", __DATE__, __TIME__);
   TRACE_INFO("Target: MPC5748G\r\n");
   TRACE_INFO("\r\n");

   //Configure I/Os
   ioInit();

   //TCP/IP stack initialization
   error = netInit();
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize TCP/IP stack!\r\n");
   }

   //Configure the first Ethernet interface
   interface = &netInterface[0];

   //Set interface name
   netSetInterfaceName(interface, APP_IF_NAME);
   //Set host name
   netSetHostname(interface, APP_HOST_NAME);
   //Set host MAC address
   macStringToAddr(APP_MAC_ADDR, &macAddr);
   netSetMacAddr(interface, &macAddr);
   //Select the relevant network adapter
   netSetDriver(interface, &mpc5748Eth1Driver);
   netSetPhyDriver(interface, &ksz8081PhyDriver);

   //Initialize network interface
   error = netConfigInterface(interface);
   //Any error to report?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to configure interface %s!\r\n", interface->name);
   }

#if (IPV4_SUPPORT == ENABLED)
#if (APP_USE_DHCP_CLIENT == ENABLED)
   //Get default settings
   dhcpClientGetDefaultSettings(&dhcpClientSettings);
   //Set the network interface to be configured by DHCP
   dhcpClientSettings.interface = interface;
   //Disable rapid commit option
   dhcpClientSettings.rapidCommit = FALSE;

   //DHCP client initialization
   error = dhcpClientInit(&dhcpClientContext, &dhcpClientSettings);
   //Failed to initialize DHCP client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize DHCP client!\r\n");
   }

   //Start DHCP client
   error = dhcpClientStart(&dhcpClientContext);
   //Failed to start DHCP client?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start DHCP client!\r\n");
   }
#else
   //Set IPv4 host address
   ipv4StringToAddr(APP_IPV4_HOST_ADDR, &ipv4Addr);
   ipv4SetHostAddr(interface, ipv4Addr);

   //Set subnet mask
   ipv4StringToAddr(APP_IPV4_SUBNET_MASK, &ipv4Addr);
   ipv4SetSubnetMask(interface, ipv4Addr);

   //Set default gateway
   ipv4StringToAddr(APP_IPV4_DEFAULT_GATEWAY, &ipv4Addr);
   ipv4SetDefaultGateway(interface, ipv4Addr);

   //Set primary and secondary DNS servers
   ipv4StringToAddr(APP_IPV4_PRIMARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 0, ipv4Addr);
   ipv4StringToAddr(APP_IPV4_SECONDARY_DNS, &ipv4Addr);
   ipv4SetDnsServer(interface, 1, ipv4Addr);
#endif
#endif

#if (IPV6_SUPPORT == ENABLED)
#if (APP_USE_SLAAC == ENABLED)
   //Get default settings
   slaacGetDefaultSettings(&slaacSettings);
   //Set the network interface to be configured
   slaacSettings.interface = interface;

   //SLAAC initialization
   error = slaacInit(&slaacContext, &slaacSettings);
   //Failed to initialize SLAAC?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize SLAAC!\r\n");
   }

   //Start IPv6 address autoconfiguration process
   error = slaacStart(&slaacContext);
   //Failed to start SLAAC process?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start SLAAC!\r\n");
   }
#else
   //Set link-local address
   ipv6StringToAddr(APP_IPV6_LINK_LOCAL_ADDR, &ipv6Addr);
   ipv6SetLinkLocalAddr(interface, &ipv6Addr);

   //Set IPv6 prefix
   ipv6StringToAddr(APP_IPV6_PREFIX, &ipv6Addr);
   ipv6SetPrefix(interface, 0, &ipv6Addr, APP_IPV6_PREFIX_LENGTH);

   //Set global address
   ipv6StringToAddr(APP_IPV6_GLOBAL_ADDR, &ipv6Addr);
   ipv6SetGlobalAddr(interface, 0, &ipv6Addr);

   //Set default router
   ipv6StringToAddr(APP_IPV6_ROUTER, &ipv6Addr);
   ipv6SetDefaultRouter(interface, 0, &ipv6Addr);

   //Set primary and secondary DNS servers
   ipv6StringToAddr(APP_IPV6_PRIMARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 0, &ipv6Addr);
   ipv6StringToAddr(APP_IPV6_SECONDARY_DNS, &ipv6Addr);
   ipv6SetDnsServer(interface, 1, &ipv6Addr);
#endif
#endif


   //Get default settings
   httpServerGetDefaultSettings(&httpServerSettings);
   //Bind HTTP server to the desired interface
   httpServerSettings.interface = &netInterface[0];
   //Listen to port 80
   httpServerSettings.port = HTTP_PORT;
   //Client connections
   httpServerSettings.maxConnections = APP_HTTP_MAX_CONNECTIONS;
   httpServerSettings.connections = httpConnections;
   //Specify the server's root directory
   strcpy(httpServerSettings.rootDirectory, "/www/");
   //Set default home page
   strcpy(httpServerSettings.defaultDocument, "index.shtm");
   //Callback functions
   httpServerSettings.cgiCallback = httpServerCgiCallback;
   httpServerSettings.requestCallback = httpServerRequestCallback;
   httpServerSettings.uriNotFoundCallback = httpServerUriNotFoundCallback;

   //HTTP server initialization
   error = httpServerInit(&httpServerContext, &httpServerSettings);
   //Failed to initialize HTTP server?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to initialize HTTP server!\r\n");
   }

   //Start HTTP server
   error = httpServerStart(&httpServerContext);
   //Failed to start HTTP server?
   if(error)
   {
      //Debug message
      TRACE_ERROR("Failed to start HTTP server!\r\n");
   }

   DisableResetEscalation();
   UDS_Init(&a_BLHandlersConfig);
   Bootloader_Init(&a_BLHandlersConfig);


   //Set task parameters
   taskParams = OS_TASK_DEFAULT_PARAMS;
   taskParams.stackSize = 200;
   taskParams.priority = OS_TASK_PRIORITY_NORMAL;

   //Create a task to blink the LED
   taskId = osCreateTask("LED", ledTask, NULL, &taskParams);
   //Failed to create the task?
   if(taskId == OS_INVALID_TASK_ID)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }

   // DOIP task create
   taskParams = OS_TASK_DEFAULT_PARAMS;
   taskParams.stackSize = 4096;                 // DoIP parsing + buffers, safer than 500
   taskParams.priority = OS_TASK_PRIORITY_NORMAL;

   taskId = osCreateTask("DoIP TCP", DoIP_TcpServerTask, NULL, &taskParams);
   if(taskId == OS_INVALID_TASK_ID)
   {
      TRACE_ERROR("Failed to create DoIP TCP task!\r\n");
   }

   //Start the execution of tasks
   osStartKernel();

   //This function should never return
   return 0;
}


/**
 * @brief CGI callback function
 * @param[in] connection Handle referencing a client connection
 * @param[in] param NULL-terminated string that contains the CGI parameter
 * @return Error code
 **/

error_t httpServerCgiCallback(HttpConnection *connection,
   const char_t *param)
{
   static uint_t pageCounter = 0;
   uint_t length;
   MacAddr macAddr;
#if (IPV4_SUPPORT == ENABLED)
   Ipv4Addr ipv4Addr;
#endif
#if (IPV6_SUPPORT == ENABLED)
   uint_t n;
   Ipv6Addr ipv6Addr;
#endif

   //Underlying network interface
   NetInterface *interface = connection->socket->interface;

   //Check parameter name
   if(!strcasecmp(param, "PAGE_COUNTER"))
   {
      pageCounter++;
      sprintf(connection->buffer, "%u time%s", pageCounter, (pageCounter >= 2) ? "s" : "");
   }
   else if(!strcasecmp(param, "BOARD_NAME"))
   {
      strcpy(connection->buffer, "DEVKIT-MPC5748G");
   }
   else if(!strcasecmp(param, "SYSTEM_TIME"))
   {
      systime_t time = osGetSystemTime();
      formatSystemTime(time, connection->buffer);
   }
   else if(!strcasecmp(param, "MAC_ADDR"))
   {
      netGetMacAddr(interface, &macAddr);
      macAddrToString(&macAddr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_ADDR"))
   {
      ipv4GetHostAddr(interface, &ipv4Addr);
      ipv4AddrToString(ipv4Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "SUBNET_MASK"))
   {
      ipv4GetSubnetMask(interface, &ipv4Addr);
      ipv4AddrToString(ipv4Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "DEFAULT_GATEWAY"))
   {
      ipv4GetDefaultGateway(interface, &ipv4Addr);
      ipv4AddrToString(ipv4Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_PRIMARY_DNS"))
   {
      ipv4GetDnsServer(interface, 0, &ipv4Addr);
      ipv4AddrToString(ipv4Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV4_SECONDARY_DNS"))
   {
      ipv4GetDnsServer(interface, 1, &ipv4Addr);
      ipv4AddrToString(ipv4Addr, connection->buffer);
   }
#if (IPV6_SUPPORT == ENABLED)
   else if(!strcasecmp(param, "LINK_LOCAL_ADDR"))
   {
      ipv6GetLinkLocalAddr(interface, &ipv6Addr);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "GLOBAL_ADDR"))
   {
      ipv6GetGlobalAddr(interface, 0, &ipv6Addr);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_PREFIX"))
   {
      ipv6GetPrefix(interface, 0, &ipv6Addr, &n);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
      length = strlen(connection->buffer);
      sprintf(connection->buffer + length, "/%u", n);
   }
   else if(!strcasecmp(param, "ROUTER"))
   {
      ipv6GetDefaultRouter(interface, 0, &ipv6Addr);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_PRIMARY_DNS"))
   {
      ipv6GetDnsServer(interface, 0, &ipv6Addr);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
   }
   else if(!strcasecmp(param, "IPV6_SECONDARY_DNS"))
   {
      ipv6GetDnsServer(interface, 1, &ipv6Addr);
      ipv6AddrToString(&ipv6Addr, connection->buffer);
   }
#endif
   else
   {
      return ERROR_INVALID_TAG;
   }

   //Get the length of the resulting string
   length = strlen(connection->buffer);

   //Send the contents of the specified environment variable
   return httpWriteStream(connection, connection->buffer, length);
}


/**
 * @brief HTTP request callback
 * @param[in] connection Handle referencing a client connection
 * @param[in] uri NULL-terminated string containing the path to the requested resource
 * @return Error code
 **/

error_t httpServerRequestCallback(HttpConnection *connection,
   const char_t *uri)
{
   //Not implemented
   return ERROR_NOT_FOUND;
}


/**
 * @brief URI not found callback
 * @param[in] connection Handle referencing a client connection
 * @param[in] uri NULL-terminated string containing the path to the requested resource
 * @return Error code
 **/

error_t httpServerUriNotFoundCallback(HttpConnection *connection,
   const char_t *uri)
{
   //Not implemented
   return ERROR_NOT_FOUND;
}
