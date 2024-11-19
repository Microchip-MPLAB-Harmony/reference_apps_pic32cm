/*******************************************************************************
  MPLAB Harmony Application Source File

  Company:
    Microchip Technology Inc.

  File Name:
    app.c

  Summary:
    This file contains the source code for the MPLAB Harmony application.

  Description:
    This file contains the source code for the MPLAB Harmony application.  It
    implements the logic of the application's state machine and it may call
    API routines of other MPLAB Harmony modules in the system, such as drivers,
    system services, and middleware.  However, it does not call any of the
    system interfaces (such as the "Initialize" and "Tasks" functions) of any of
    the modules in the system or make any assumptions about when those functions
    are called.  That is the responsibility of the configuration-specific system
    files.
 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2024 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
//DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************

#include "app.h"
#include "user.h"
#include "stdio.h"
// *****************************************************************************
// *****************************************************************************
// Section: Global Data Definitions
// *****************************************************************************
// *****************************************************************************

#define SDCARD_MOUNT_NAME    "/mnt/myDrive1"
#define SDCARD_DEV_NAME      "/dev/mmcblka1"
#define SDCARD_FILE_NAME     "Text_File.txt"
#define SDCARD_DIR_NAME      "Dir1"
#define APP_DATA_LEN         512

// *****************************************************************************
/* Application Data

  Summary:
    Holds application data

  Description:
    This structure holds the application's data.

  Remarks:
    This structure should be initialized by the APP_Initialize function.

    Application strings and buffers are be defined outside this structure.
*/

APP_DATA appData;

uint8_t writeData[12] = "Hello World";


// *****************************************************************************
// *****************************************************************************
// Section: Application Callback Functions
// *****************************************************************************
// *****************************************************************************

/* TODO:  Add any necessary callback functions.
*/

// *****************************************************************************
// *****************************************************************************
// Section: Application Local Functions
// *****************************************************************************
// *****************************************************************************


/* TODO:  Add any necessary local functions.
*/


// *****************************************************************************
// *****************************************************************************
// Section: Application Initialization and State Machine Functions
// *****************************************************************************
// *****************************************************************************

/*******************************************************************************
  Function:
    void APP_Initialize ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Initialize ( void )
{   
    
    printf("\n\n\r-------------------------------------------------------------");
    printf("\n\r       SD Card Example Application on PIC32CM LS00 Cnano      ");
    printf("\n\r-------------------------------------------------------------");

     /* Indicating that the process is beginning */
    printf("\n\n\r  Starting the process..."); 
    
    /*Press the SWITCH to initiate the process*/
    printf("\n\n\r  Press the SWITCH to initiate the process ");
    
    /* Place the App state machine in its initial state. */
    appData.state = APP_WAIT_SWITCH_PRESS; 
    
    /* Wait for the switch status to return to the default "not pressed" state */
    while(SWITCH1_Get() == SWITCH_STATUS_PRESSED);
    
    
}


/******************************************************************************
  Function:
    void APP_Tasks ( void )

  Remarks:
    See prototype in app.h.
 */

void APP_Tasks ( void )
{
    /* Check the application's current state. */
    switch ( appData.state )
    {
        case APP_WAIT_SWITCH_PRESS:
              
            /*Press the SWITCH to initiate the process*/
            if (SWITCH1_Get() == SWITCH_STATUS_PRESSED)
            {
                appData.state = APP_MOUNT_DISK;
            
            }
            break;
            
        case APP_MOUNT_DISK:
       
            if(SYS_FS_Mount(SDCARD_DEV_NAME, SDCARD_MOUNT_NAME, FAT, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try
                 * mounting again until success. */
                appData.state = APP_MOUNT_DISK;

            }
            else
            {
                /* Mount was successful. Unmount the disk, for testing. */
                appData.state = APP_UNMOUNT_DISK;
                printf("\n\n\r  Stage 1: SD card mounted successfully");               
            }
            break;

        case APP_UNMOUNT_DISK:
          
            if(SYS_FS_Unmount(SDCARD_MOUNT_NAME) != 0)
            {
                /* The disk could not be un mounted. Try
                 * un mounting again until success. */

                appData.state = APP_UNMOUNT_DISK;
     
            }
            else
            {
                /* UnMount was successful. Mount the disk again */
                printf("\n\r  Stage 2: SD card unmounted successfully");
                appData.state = APP_MOUNT_DISK_AGAIN;
              
            }
            break;

        case APP_MOUNT_DISK_AGAIN:
           
            if(SYS_FS_Mount(SDCARD_DEV_NAME, SDCARD_MOUNT_NAME, FAT, 0, NULL) != 0)
            {
                /* The disk could not be mounted. Try
                 * mounting again until success. */
                appData.state = APP_MOUNT_DISK_AGAIN;
            }
            else
            {
                /* Mount was successful. Set current drive so that we do not have to use absolute path. */
                appData.state = APP_SET_CURRENT_DRIVE;
                
                printf("\n\r  Stage 3: Current drive set. Ready for file operations");
            }
            break;

        case APP_SET_CURRENT_DRIVE: 
         
            if(SYS_FS_CurrentDriveSet(SDCARD_MOUNT_NAME) == SYS_FS_RES_FAILURE)
            {
                //directly move to APP_CREATE_DIRECTORY case
                appData.state = APP_CREATE_DIRECTORY;
                printf("\n\r  Stage 4: New directory created on the SD card");
            }
            else
            {
                //directly move to APP_CREATE_DIRECTORY case
                appData.state = APP_CREATE_DIRECTORY;
                printf("\n\r  Stage 4: New directory created on the SD card");
            }
            break;

        case APP_CREATE_DIRECTORY:
        
            if(SYS_FS_DirectoryMake(SDCARD_DIR_NAME) == SYS_FS_RES_FAILURE)
            {
                printf("\n\r  Stage 5: The file has been opened. It already exists");
                //if directory already exists Then write to file//
                appData.state = APP_OPEN_THE_FILE;
            }
            else
            {
                printf("\n\r  Stage 5: File opened");
               //if directory doesn't exists its created and Then write to file//
                appData.state = APP_OPEN_THE_FILE;
            }
            break;

        case APP_OPEN_THE_FILE: //not used state move to next case//
            
            appData.fileHandle=SYS_FS_FileOpen(SDCARD_DIR_NAME"/"SDCARD_FILE_NAME, (SYS_FS_FILE_OPEN_APPEND_PLUS));
            if(appData.fileHandle == SYS_FS_HANDLE_INVALID)
            {
                /* Could not open the file. Error out*/
                appData.state = APP_ERROR;

            }
            else
            {
                /* File opened successfully. Write to file */
                appData.state = APP_WRITE_TO_FILE;
                printf("\n\r  Stage 6: Data written to the file");
            }
            break;
            
        case APP_WRITE_TO_FILE: //case used to write data into the file//
            
            if (SYS_FS_FileWrite( appData.fileHandle, (const void *) writeData, 12 ) == -1)
            {
                /* Write was not successful. Close the file
                 * and error out.*/
                SYS_FS_FileClose(appData.fileHandle);
                appData.state = APP_ERROR;

            }
            else
            {
                /* We are done writing. Close the file */
                appData.state = APP_CLOSE_FILE;
                printf("\n\r  Stage 7: File closed");
            }

            break;


        case APP_CLOSE_FILE:
            
            SYS_FS_FileClose(appData.fileHandle);
           
            appData.state = APP_IDLE;
            printf("\n\r  Stage 8: System is idle");
            
            /* SDCARD file writing is success */
            printf("\n\n\n\r  SD Card file writing is successful..!");
            break;

        case APP_IDLE:
            
            LED1_Set(); 

            break;

        case APP_ERROR:
          
            /* The application comes here when the demo has failed. */
            
            LED1_Clear();
            if(SYS_FS_Unmount(SDCARD_MOUNT_NAME) != 0)
            {
                /* The disk could not be un mounted. Try
                 * un mounting again until success. */
                
                appData.state = APP_ERROR;
            }
            break;

        default:
            break;
    }
}

/*******************************************************************************
 End of File
 */
