set(HEADERS 
  AttributeUtils.hxx
  Constants.hxx
  EventUtils.hxx
  make_writer.hxx
  ReaderUtils.hxx
  Traits.hxx
  Types.hxx
  UnitsUtils.hxx
  WriterUtils.hxx
  Exceptions.hxx
  FATXUtils.hxx)

set(IMPLEMENTATION 
  EventUtils.cxx
  make_writer.cxx
  ReaderUtils.cxx
  WriterUtils.cxx
  UnitsUtils.cxx
  FATXUtils.cxx)

add_library(nuhepmc_cpputils SHARED ${IMPLEMENTATION})
target_link_libraries(nuhepmc_cpputils PUBLIC NuHepMC::Options)
target_link_libraries(nuhepmc_cpputils PRIVATE fmt)
target_compile_definitions(nuhepmc_cpputils PRIVATE FMT_HEADER_ONLY)
set_target_properties(nuhepmc_cpputils PROPERTIES 
  PUBLIC_HEADER "${HEADERS}"
  EXPORT_NAME CPPUtils)

install(TARGETS nuhepmc_cpputils
    EXPORT nuhepmc_cpputils-targets
    LIBRARY DESTINATION lib/
    PUBLIC_HEADER DESTINATION include/NuHepMC)

add_library(NuHepMC::CPPUtils ALIAS nuhepmc_cpputils)

if(NuHepMC_BUILD_STATIC_LIBS)
  add_library(nuhepmc_cpputils_static STATIC ${IMPLEMENTATION})
  target_link_libraries(nuhepmc_cpputils_static PUBLIC NuHepMC::Options_static)
  target_link_libraries(nuhepmc_cpputils_static PRIVATE fmt)
  target_compile_definitions(nuhepmc_cpputils_static PRIVATE FMT_HEADER_ONLY)

  set_target_properties(nuhepmc_cpputils_static PROPERTIES 
    EXPORT_NAME CPPUtils_static)

  install(TARGETS nuhepmc_cpputils_static
      EXPORT nuhepmc_cpputils-targets
      LIBRARY DESTINATION lib/)

  add_library(NuHepMC::CPPUtils_static ALIAS nuhepmc_cpputils_static)

endif()

