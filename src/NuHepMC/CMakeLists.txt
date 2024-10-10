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
target_link_libraries(nuhepmc_cpputils PRIVATE fmt::fmt-header-only nuhepmc_private_compile_options)
set_target_properties(nuhepmc_cpputils PROPERTIES 
  PUBLIC_HEADER "${HEADERS}"
  EXPORT_NAME CPPUtils)

install(TARGETS nuhepmc_cpputils
    EXPORT nuhepmc_cpputils-targets
    LIBRARY DESTINATION lib/
    PUBLIC_HEADER DESTINATION include/NuHepMC)

add_library(NuHepMC::CPPUtils ALIAS nuhepmc_cpputils)

