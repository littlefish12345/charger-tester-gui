#----------------------------------------------------------------
# Generated CMake target import file.
#----------------------------------------------------------------

# Commands may need to know the format version.
set(CMAKE_IMPORT_FILE_VERSION 1)

# Import target "ElaWidgetTools" for configuration ""
set_property(TARGET ElaWidgetTools APPEND PROPERTY IMPORTED_CONFIGURATIONS NOCONFIG)
set_target_properties(ElaWidgetTools PROPERTIES
  IMPORTED_LOCATION_NOCONFIG "${_IMPORT_PREFIX}/ElaWidgetTools/lib/libElaWidgetTools.so"
  IMPORTED_SONAME_NOCONFIG "libElaWidgetTools.so"
  )

list(APPEND _cmake_import_check_targets ElaWidgetTools )
list(APPEND _cmake_import_check_files_for_ElaWidgetTools "${_IMPORT_PREFIX}/ElaWidgetTools/lib/libElaWidgetTools.so" )

# Commands beyond this point should not need to know the version.
set(CMAKE_IMPORT_FILE_VERSION)
