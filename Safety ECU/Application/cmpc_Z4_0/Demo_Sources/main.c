/**
 * @file main.c
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
#include "feature_management.h"
#include "bsd_receiver_task.h"   /* just declares bsdReceiverTaskCreate() */

// #include "socket.h"



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


typedef enum
{
   DIAG_STATE_IDLE = 0,
   DIAG_STATE_MAIN_MENU = 1,
   DIAG_STATE_ENGINE_MENU = 2,
   DIAG_STATE_TRANSMISSION_MENU = 3,
   DIAG_STATE_RUNNING_TEST = 4
} DiagnosticState;


//Handled in DOIP
//Socket *tcpServerSocket = NULL;
//uint8_t tcpBuffer[APP_TCP_BUFFER_SIZE];

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

#define TASK_ID_FEATURE_X   1
#define TASK_ID_FEATURE_Y   3
#define TASK_ID_FEATURE_BSD 2


extern Feature_manager gFeatureManager;
extern volatile bool gFeatureFlashBusy;



static bool is_feature_active(Feature_manager *fm, uint16_t task_id)
{
   Feature *feature = get_feature_by_task_id(fm, task_id);

   if (feature == NULL)
      return false;

   if (feature->is_enabled != true)
      return false;

   if (feature->state != FEATURE_ENABLED)
      return false;

   return true;
}
void ledTask(void *param)
{
   //Endless loop
   while(1)
   {
      SIUL2->GPDO[USR_LED2] = 0;
      SIUL2->GPDO[USR_LED3] = 0;
   }
}


void feature_xTask(void *param)
{
   (void)param;

   while(1)
   {
      if (gFeatureFlashBusy)
      {
         SIUL2->GPDO[USR_LED5] = 1;
         osDelayTask(50);
         continue;
      }

      if (is_feature_active(&gFeatureManager, TASK_ID_FEATURE_X))
      {
         SIUL2->GPDO[USR_LED5] = 0;
         osDelayTask(100);
         SIUL2->GPDO[USR_LED5] = 1;
         osDelayTask(100);
      }
      else
      {
         SIUL2->GPDO[USR_LED5] = 1;
         osDelayTask(500);
      }
   }
}



void feature_yTask(void *param)
{
   (void)param;

   while(1)
   {
      if (gFeatureFlashBusy)
      {
         SIUL2->GPDO[USR_LED4] = 1;
         osDelayTask(50);
         continue;
      }

      if (is_feature_active(&gFeatureManager, TASK_ID_FEATURE_Y))
      {
         SIUL2->GPDO[USR_LED4] = 0;
         osDelayTask(700);
         SIUL2->GPDO[USR_LED4] = 1;
         osDelayTask(700);

      }
      else
      {
         SIUL2->GPDO[USR_LED4] = 1;
         osDelayTask(500);
      }
   }
}
static bool isBSD = false;

void feature_BSDTask(void *param)
{
    (void)param;

    while(1)
    {
        bool enabled =
            is_feature_active(&gFeatureManager,
                              TASK_ID_FEATURE_BSD);

        if(enabled && !isBSD)
        {
            isBSD = true;

            TRACE_INFO("BSD Enabled\r\n");

            bsdReceiverTaskCreate();
        }
        else if(!enabled && isBSD)
        {
            isBSD = false;

            TRACE_INFO("BSD Disabled\r\n");

            bsdReceiverTaskDelete();
        }

        osDelayTask(100);
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


void Test_FeatureManagerFlash(void)
{
    Feature_manager fmWrite;
    Feature_manager fmRead;
    status_r status;

    memset(&fmWrite, 0, sizeof(fmWrite));
    memset(&fmRead, 0, sizeof(fmRead));

    /* 1. Initialize default feature manager */
    init_features(&fmWrite);

    /* 2. Modify test data */
    fmWrite.feature_count = 2U;

    fmWrite.features[0].id = 1U;
    strncpy(fmWrite.features[0].name, "ECF", sizeof(fmWrite.features[0].name) - 1U);
    fmWrite.features[0].name[sizeof(fmWrite.features[0].name) - 1U] = '\0';
    fmWrite.features[0].is_enabled = true;
    fmWrite.features[0].valid_until = 1500000000UL;
    fmWrite.features[0].state = FEATURE_ENABLED;

    fmWrite.features[1].id = 2U;
    strncpy(fmWrite.features[1].name, "ACC", sizeof(fmWrite.features[1].name) - 1U);
    fmWrite.features[1].name[sizeof(fmWrite.features[1].name) - 1U] = '\0';
    fmWrite.features[1].is_enabled = false;
    fmWrite.features[1].valid_until = 1800000000UL;
    fmWrite.features[1].state = FEATURE_UNAVAILABLE;

    /* 3. Save to flash */
    status = Feature_manager_save_to_flash(&fmWrite);

    if (status != WRITE_SUCCESS)
    {
        TRACE_INFO("Feature Manager save FAILED\r\n");
        return;
    }

    TRACE_INFO("Feature Manager save SUCCESS\r\n");

    /* 4. Clear RAM to prove load is from flash */
    memset(&fmRead, 0, sizeof(fmRead));
    TRACE_INFO("AFTER CLEARING RAM VERSION Feature 0: id=%d name=%s enabled=%d valid_until=%lu state=%d\r\n",
                fmRead.features[0].id,
                fmRead.features[0].name,
                fmRead.features[0].is_enabled,
                fmRead.features[0].valid_until,
                fmRead.features[0].state);

    /* 5. Load from flash */
    status = Feature_manager_load_from_flash(&fmRead);

    if (status != READ_SUCCESS)
    {
        TRACE_INFO("Feature Manager load FAILED\r\n");
        return;
    }

    TRACE_INFO("Feature Manager load SUCCESS\r\n");

    /* 6. Compare written and loaded data */
    if (memcmp(&fmWrite, &fmRead, sizeof(Feature_manager)) == 0)
    {
        TRACE_INFO("Feature Manager flash TEST PASSED\r\n");
    }
    else
    {
        TRACE_INFO("Feature Manager flash TEST FAILED: data mismatch\r\n");
    }

    /* 7. Print loaded data */
    TRACE_INFO("Loaded feature count = %d\r\n", fmRead.feature_count);

    TRACE_INFO("Feature 0: id=%d name=%s enabled=%d valid_until=%lu state=%d\r\n",
               fmRead.features[0].id,
               fmRead.features[0].name,
               fmRead.features[0].is_enabled,
               fmRead.features[0].valid_until,
               fmRead.features[0].state);

    TRACE_INFO("Feature 1: id=%d name=%s enabled=%d valid_until=%lu state=%d\r\n",
               fmRead.features[1].id,
               fmRead.features[1].name,
               fmRead.features[1].is_enabled,
               fmRead.features[1].valid_until,
               fmRead.features[1].state);
}



int_t main(void)
{
   error_t error;
   OsTaskId taskId,feature_Xhandle,feature_Yhandle,bsdTasK_Handle;


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

   //calling the functions of feature manager flashing
  // Test_FeatureManagerFlash(); //added 28/4/2026 for testing the read and write of the image


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
   TRACE_INFO("public key parsing before the ethernet inits \r\n");
   Signature_PublicKey_Init();


   status_r fmStatus;

   fmStatus = Feature_manager_load_from_flash(&gFeatureManager);

   TRACE_INFO("gFeatureManager count=%d\r\n", gFeatureManager.feature_count);

   for (uint8_t i = 0; i < gFeatureManager.feature_count; i++)
   {
       TRACE_INFO("Feature[%d]: id=%d name=%s enabled=%d state=%d valid_until=%lu\r\n",
                  i,
                  gFeatureManager.features[i].id,
                  gFeatureManager.features[i].name,
				  gFeatureManager.features[i].task_id,
                  gFeatureManager.features[i].is_enabled,
                  gFeatureManager.features[i].state,
                  gFeatureManager.features[i].valid_until);
   }

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


      /******************************************************************************/

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

   // to mimic the features
   feature_Xhandle=osCreateTask("LED3", feature_xTask, NULL, &taskParams);
   feature_Yhandle=osCreateTask("LED4", feature_yTask, NULL, &taskParams);
  // feature_Zhandle=osCreateTask("LED5", feature_zTask, NULL, &taskParams);
   bsdTasK_Handle=osCreateTask("BSD", feature_BSDTask, NULL, &taskParams);
		   // create bsd task

   //Failed to create the task?
   if(taskId == OS_INVALID_TASK_ID)
   {
      //Debug message
      TRACE_ERROR("Failed to create task!\r\n");
   }



   TRACE_INFO("signature verification testing\r\n");
   Test_PublicKey_And_Signature();

   // DOIP task
   taskParams = OS_TASK_DEFAULT_PARAMS;
   taskParams.stackSize = 8192;                 // DoIP parsing + buffers, safer than 500
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
