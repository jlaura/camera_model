message( "External project - Spice" )

# This is hard coded for linux 64 right now, but should be generalized for all OSes.
ExternalProject_Add( Spice
  URL "http://naif.jpl.nasa.gov/pub/naif/toolkit//C/PC_Linux_GCC_64bit/packages/cspice.tar.Z"
  DOWNLOAD_NAME cspice.tar.gz
  UPDATE_COMMAND ""
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND
    ${CMAKE_COMMAND} -E copy
      ${CMAKE_BINARY_DIR}/Spice-prefix/src/Spice/lib/cspice.a
      ${CMAKE_BINARY_DIR}/Spice-prefix/src/Spice/lib/libcspice.a &&
    ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_BINARY_DIR}/Spice-prefix/src/Spice/
      ${INSTALL_DEPENDENCIES_DIR}/cspice
)