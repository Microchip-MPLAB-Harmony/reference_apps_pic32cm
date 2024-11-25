---
grand_parent: PIC32CM Reference Applications
parent: PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit
title: SD Card
nav_order: 2
---

<img src = "images/microchip_logo.png">
<img src = "images/microchip_mplab_harmony_logo_small.png">

# SD Card Example Application on PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit
<h2 align="center"> <a href="https://github.com/Microchip-MPLAB-Harmony/reference_apps/releases/latest/download/pic32cm_ls00_cnano_sdcard.zip" > Download </a> </h2>

-----
## Description:

> This application demonstrates the use of the File System to access and modify a text file on an SD card using the SDSPI driver with the Curiosity Nano Explorer board and the PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit.

## Modules/Technology Used:
- Peripheral Modules
	- TIME
	- SERCOM (USART)
	- SERCOM (SPI)

- Driver Modules
	- SPI
	- SD CARD (SPI) 

- System Services
	- File System

- Core

	The MCC Harmony project graph with all the components would look like this:

    <img src = "images/project_graph.png" width="630" height="480" align="center">

## Hardware Used:

- [PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit](https://www.microchip.com/en-us/development-tool/ev41c56a)
- [Curiosity Nano Explorer](https://www.microchip.com/en-us/development-tool/EV58G97A)
- SD Card

## Software/Tools Used:
<span style="color:blue"> This project has been verified to work with the following versions of software tools:</span>  
- [MPLAB X IDE](https://www.microchip.com/en-us/tools-resources/develop/mplab-x-ide) v6.20
- [MPLAB Code Configurator Plugin](https://www.microchip.com/en-us/tools-resources/configure/mplab-code-configurator)  v5.5.1
- [MPLAB XC32 Compiler](https://www.microchip.com/en-us/tools-resources/develop/mplab-xc-compilers) v4.45
- [csp](https://github.com/Microchip-MPLAB-Harmony/csp) v3.19.7
- CMSIS_5 v5.9.0
- PIC32CM-LS_DFP v1.2.274

Refer [Project Manifest](./firmware/src/config/pic32cm_ls00_cnano/harmony-manifest-success.yml) present in harmony-manifest-success.yml under the project folder *firmware/src/config/pic32cm_ls00_cnano*  
- Refer the [Release Notes](../../../release_notes.md#development-tools) to know the **MPLAB X IDE** and **MCC** Plugin version. Alternatively, [Click Here](https://github.com/Microchip-MPLAB-Harmony/reference_apps/blob/master/release_notes.md#development-tools).
- Any Serial Terminal application like Tera Term/PuTTY terminal application.

<span style="color:blue"> Because Microchip regularly update tools, occasionally issue(s) could be discovered while using the newer versions of the tools. If the project doesn’t seem to work and version incompatibility is suspected, It is recommended to double-check and use the same versions that the project was tested with. </span> To download original version of MPLAB Harmony v3 packages, refer to document [How to Use the MPLAB Harmony v3 Project Manifest Feature](https://ww1.microchip.com/downloads/en/DeviceDoc/How-to-Use-the-MPLAB-Harmony-v3-Project-Manifest-Feature-DS90003305.pdf)

## Setup:
- Connect the PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit to the host PC using a Type-A male to micro-B USB cable. Plug the cable into the Micro-B USB (Debug USB) port on the evaluation kit.
- Make sure the SD Card is inserted into the Curiosity Nano Explorer board.


  <img src = "images/hardware_setup.jpg" width="660" height="580" align="center">

## Programming hex file:
The pre-built hex file can be programmed by following the below steps.  

### Steps to program the hex file
- Open MPLAB X IDE
- Close all existing projects in IDE, if any project is opened.
- Go to File -> Import -> Hex/ELF File
- In the "Import Image File" window, Step 1 - Create Prebuilt Project, Click the "Browse" button to select the prebuilt hex file.
- Select Device has "PIC32CM5164LS00048"
- Ensure the proper tool is selected under "Hardware Tool"
- Click on Next button
- In the "Import Image File" window, Step 2 - Select Project Name and Folder, select appropriate project name and folder
- Click on Finish button
- In MPLAB X IDE, click on "Make and Program Device" Button. The device gets programmed in sometime
- Follow the steps in "Running the Demo" section below

## Programming/Debugging Application Project:
- Open the project (pic32cm_ls00_cnano_sdcard/firmware/sdcard_pic32cm_ls00_cnanogroup) in MPLAB X IDE
- Ensure "PIC32CM LS00 Curiosity Nano" is selected as hardware tool to program/debug the application
- Build the code and program the device by clicking on the "make and program" button in MPLAB X IDE tool bar
- Follow the steps in "Running the Demo" section below

## Running the Demo:
- Open the Tera Term/PuTTY terminal application on your PC (from the Windows® Start menu by pressing the Start button)
- Set the baud rate to 115200
- To Reset the device, run this command: **ipecmd.exe -P32CM5164LS00048 -TPNEDBG -OK** from the following location: *C:/Program Files/Microchip/MPLABX/v6.20/mplab_platform/mplab_ipe*

  <img src = "images/reset_command.png" width="600" height="280" align="middle">
  
  **NOTE**: *The PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit does not include a reset button. Therefore, the device can be reset by executing the reset command in the CMD prompt.*
- The console displays the following message

	<img src = "images/initial_message.png">
- Press the switch SW1 on the PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit to perform the File operation.
- The file operation is successfully completed, and LED1 on the PIC32CM LS00 Curiosity Nano + Touch Evaluation Kit turns on. If an error occurs during the file operation, LED1 remains off.

	<img src = "images/hardware_output.jpg" width="600" height="400" align="middle">

- The following messages are displayed in the serial terminal during the file operation process when it is successful.

	<img src = "images/console_output.png">

- After receiving the **'SD Card file writing is successful..!'** message in the serial terminal, the file on the SD card is verified by inserting the SD card into a PC, opening the file, and checking its contents. This confirms that the file has been written correctly.
## Comments:
- Reference Training Module: [Arm TrustZone Getting Started Application on PIC32CM LS60 (Arm Cortex-M23) MCUs](https://developerhelp.microchip.com/xwiki/bin/view/software-tools/harmony/pic32cm-trustzone-getting-started-training-module/)
- This application demo builds and works out of box by following the instructions above in "Running the Demo" section. If you need to enhance/customize this application demo, you need to use the MPLAB Harmony v3 Software framework. Refer links below to setup and build your applications using MPLAB Harmony.
	- [How to Setup MPLAB Harmony v3 Software Development Framework](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/SupportingCollateral/How-to-Setup-MPLAB-Harmony-v3-Software-Development-Framework-DS90003232.pdf)	
	- [Video - How to Set up the Tools Required to Get Started with MPLAB® Harmony v3 and MCC](https://www.youtube.com/watch?v=0rNFSlsVwVw)	
	- [Create a new MPLAB Harmony v3 project using MCC](https://developerhelp.microchip.com/xwiki/bin/view/software-tools/harmony/getting-started-training-module-using-mcc/)
	- [Update and Configure an Existing MHC-based MPLAB Harmony v3 Project to MCC-based Project](https://developerhelp.microchip.com/xwiki/bin/view/software-tools/harmony/update-and-configure-existing-mhc-proj-to-mcc-proj/)
	- [How to Build an Application by Adding a New PLIB, Driver, or Middleware to an Existing MPLAB Harmony v3 Project](https://ww1.microchip.com/downloads/aemDocuments/documents/MCU32/ProductDocuments/SupportingCollateral/How-to-Build-an-Application-by-Adding-a-New-PLIB-Driver-or-Middleware-to-an-Existing-MPLAB-Harmony-v3-Project-DS90003253.pdf)

## Revision:
- v1.7.0 - Released demo application
