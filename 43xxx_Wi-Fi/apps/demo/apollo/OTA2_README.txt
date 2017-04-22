

 OTA2 Support notes:

 First, build the ota2_extraction application
   - This is a generic extractor needed for all OTA2 applications

 snip.ota2_extract-<PLATFORM>

 To build OTA2 version of Apollo use one of these build command lines:
  (adding ota2_factory_download, ota2_factory_image, or ota2_image indicates that it is an OTA2 build)

 demo.apollo-BCM943907WAE2_1 ota2_factory_image download download_apps run
    - builds the application and creates /build/demo.apollo-<PLATFORM>/OTA2_factory_reset_file.bin
    - does not download the factory reset image to SFLASH
      (The OTA2 Factory download takes some time. Unless you are testing Factory Reset, no reason to download)
    - downloads and runs the application

 demo.apollo-BCM943907WAE2_1 ota2_factory_download download download_apps run
    - builds the application and creates /build/demo.apollo-<PLATFORM>/OTA2_factory_reset_file.bin
    - downloads the factory reset image to SFLASH - This takes extra time
    - downloads and runs the application

 demo.apollo-BCM943907WAE2_1 ota2_image
    - builds the application and creates /build/demo.apollo-<PLATFORM>/OTA2_image_file.bin
      - This is the update file to put on your server
    - does not download the image to the SFLASH
    - does not download the application to the platform

 To use PUSH mode for updates, reset the board and hold the Factory Reset button for ~5 seconds
    You will see that the OTA2 extract application is running.
    Connect your PC's wifi to the SOFT_AP_SSID of your board defined in apps/demo/apollo/wifi_dct_config.h
    Use your Web Browser to go to 192.168.10.1

 #define SOFT_AP_SSID         "apollo"
 #define SOFT_AP_PASSPHRASE   "abcd1234"
 #define SOFT_AP_SECURITY     WICED_SECURITY_OPEN
 #define SOFT_AP_CHANNEL      149

 To PULL updates from the Web
     - adjust these lines to apps/demo/apollo/wifi_config_dct.h
     - Rebuild, then from the command line, type:
      > get_update <Server_Name | Server_IP>/<file_name>

 #define CLIENT_AP_SSID       "AP_For_Upgrade_Connection"
 #define CLIENT_AP_PASSPHRASE "AP_password"
 #define CLIENT_AP_BSS_TYPE   WICED_BSS_TYPE_INFRASTRUCTURE
 #define CLIENT_AP_SECURITY   WICED_SECURITY_WPA2_AES_PSK
 #define CLIENT_AP_CHANNEL    44
 #define CLIENT_AP_BAND       WICED_802_11_BAND_5GHZ

  For timed PULL operation, adjust these fields in apollo_start_bg_update()
     NOTE: The system uses relative times for checking updates.
           If the device is connected to an AP, you can get a network RTC and use that
           in combination with the "apollo_get_update()" function to immediately start an update.

     apollo->ota2_bg_params.initial_check_interval   = 5;         <- initial wait time (in seconds)
     apollo->ota2_bg_params.check_interval           = 10 60;   <- interval after initial wait for next check
                                                                     use SECONDS_PER_DAY for 1 day interval
     apollo->ota2_bg_params.retry_check_interval     = 5;         <- retry check if check fails

 Set the default PULL address in the Application DCT (apollo_dct.c):

     .ota2_default_update_uri        = "192.168.1.100/ota2_firmware/OTA2_image_file.bin",

     NOTE: You can change this value using code by reading and writing the Application DCT
           You can change the uri when you use the command console "get_update <uri>"

 Set the default values for these flags in the Application DCT (apollo_dct.c) based on your requirements:

    .ota2_stop_playback_to_update   = 1,
    .ota2_reboot_after_download     = 1,

 - to Factory Reset, reset the board and hold the Factory Reset button for > 10 seconds

 You can set the Version number of your software on the build line by adding

    demo.apollo-<platform> ota2_image APP_VERSION_FOR_OTA2_MAJOR=X APP_VERSION_FOR_OTA2_MINOR=Y

    These values are saved in the Application DCT and the OTA2_image_file.bin.
    They are compared in the ota2 callback if you enable the check by defining CHECK_OTA2_UPDATE_VERSION=1
    in the apollo.mk file. Comment out so that it does not check the update version to simplify development.

    #GLOBAL_DEFINES      += CHECK_OTA2_UPDATE_VERSION=1

 To see output during the download, set log level to 3
  >log 3
