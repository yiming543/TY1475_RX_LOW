# The following variables contains the files used by the different stages of the build process.
set(TY1475_RX_default_default_XC8_FILE_TYPE_assemble)
set_source_files_properties(${TY1475_RX_default_default_XC8_FILE_TYPE_assemble} PROPERTIES LANGUAGE ASM)
set(TY1475_RX_default_default_XC8_FILE_TYPE_assemblePreprocess)
set_source_files_properties(${TY1475_RX_default_default_XC8_FILE_TYPE_assemblePreprocess} PROPERTIES LANGUAGE ASM)
set(TY1475_RX_default_default_XC8_FILE_TYPE_compile
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../main.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/device_config.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/eccp3.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/epwm2.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/interrupt_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/mcc.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/pin_manager.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/tmr0.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/tmr1.c"
    "${CMAKE_CURRENT_SOURCE_DIR}/../../../mcc_generated_files/tmr2.c")
set_source_files_properties(${TY1475_RX_default_default_XC8_FILE_TYPE_compile} PROPERTIES LANGUAGE C)
set(TY1475_RX_default_default_XC8_FILE_TYPE_link)
set(TY1475_RX_default_image_name "default.elf")


# The output directory of the final image.
set(TY1475_RX_default_output_dir "${CMAKE_CURRENT_SOURCE_DIR}/../../../out/TY1475_RX")
