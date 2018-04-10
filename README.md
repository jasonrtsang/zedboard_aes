# AES Encryption/ Decryption on ZedBoard

**ONLY compatible with Vivado 2017.3**

**boot.bin is prebuild and available within `./src` if you wish to just load the ZedBoard AES system from an SD card**

#### Vivado File Structure
* `./ip_repo` (if exists, holds custom IP blocks)
* `./src` (files specified within build.tcl)
  *  Files specified within `build.tcl`
  *  Usually block designs (BD) constraints
  *  `/sources_1`
  *  `/constrs_1`
* `./build.tcl` (generated from Write Project to Tcl...)
* `./build.bat` (launcher for `build.tcl`)

#### SDK File Structure
* `./sdk`
  * Project files to import after setup 
  * Contains application and board support packages (BSP)

## Setup and Run
* Run `build.bat`
* Open `zedboard_aes.xpr`
* Generate Bitstream and Export Hardware
* Launch SDK
* Import project files from `./sdk`

## Updating Files

#### Changes to Hardware
* File -> Write Project to Tcl…
* Select Recreate Block Designs using Tcl
* Diff `build.tcl` with original
  * Copy files required into `./ip_repo` or `./src` and update paths
  * Set dynamic origin directory as so below: 

   *Edit*:
   ```
   # Set the reference directory for source  file relative paths (by default the value is  script directory path)
   set origin_dir [file dirname [info script]]
   ```
   ```
   # Create project
   create_project ${project_name} ${origin_dir} -part xc7z020clg484-1
   ```
   *Remove*:
   ```
   # Set the directory path for the original project from where this script was exported
   set orig_proj_dir "[file normalize "$origin_dir/zedboard_aes"]"
   ```

#### Changes to Software
* SDK project files should be all saved within `./sdk`
  * Any new projects within the SDK should be saved here 
