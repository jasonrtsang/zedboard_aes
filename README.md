# AES Encryption/ Decryption on ZedBoard

**ONLY compatible with Vivado 2017.3**

#### Vivado File Structure
* `./ip_repo` (custom hardware IP blocks)
* `./sdk` (custom baremetal applications)
* `./src` (support files specified within build.tcl)
* `./build.tcl` (generated from Write Project to Tcl...)
* `./build.bat` (launcher for `build.tcl`)

#### SDK File Structure
  * Project files to import after setup 
* `./sdk/fsbl`
  * First stage bootloader for boot.bin image
* `./sdk/main_cpu0`
  * Main application, kicks starts CPU1 after bootup
  * Onboard SD AES Encryption/ Decryption
  * External Ethernet Mode AES Encryption/ Decryption
  * Reformatting SD Card
* `./sdk/main_cpu1`
  * Runs the progress status during SD AES Encryption/ Decryption
* `./sdk/standalone_bsp_cpu0`
  * CPU0 board support package
  * Includes lwip and xilffs libraries
* `./sdk/standalone_bsp_cpi1`
  * CPU0 board support package

## Setup and Run
* Run `build.bat`
* Open `zedboard_aes.xpr`
* Generate Bitstream and Export Hardware
* Launch SDK
* Import project files from `./sdk`
## OR
* Copy `boot.bin` within `./src` to an SD card and bridge pins JP9 and JP10
  * Built using Create Zynq Boot Image
  * Layering:
    * fsbl
    * system_wrapper.bit
    * main_cpu0.elf
    * main_cpu1.elf