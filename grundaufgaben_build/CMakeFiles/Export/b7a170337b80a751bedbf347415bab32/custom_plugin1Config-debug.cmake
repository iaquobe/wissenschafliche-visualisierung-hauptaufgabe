#----------------------------------------------------------------
# Generated CMake target import file for configuration "Debug".
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "custom_plugin1" for configuration "Debug"
set_property(TARGET custom_plugin1 APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
set_target_properties(custom_plugin1 PROPERTIES
  IMPORTED_LOCATION_DEBUG "${_IMPORT_PREFIX}/../Fantom/lib/fantom-plugins/custom/libplugin1.so"
  IMPORTED_SONAME_DEBUG "libplugin1.so"
  )

list(APPEND _cmake_import_check_targets custom_plugin1 )
list(APPEND _cmake_import_check_files_for_custom_plugin1 "${_IMPORT_PREFIX}/../Fantom/lib/fantom-plugins/custom/libplugin1.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
