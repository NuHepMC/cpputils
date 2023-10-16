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
  CrossSectionUtils.hxx
  DistributionUtils.hxx)

set(IMPLEMENTATION 
  EventUtils.cxx
  make_writer.cxx
  ReaderUtils.cxx
  WriterUtils.cxx
  CrossSectionUtils.cxx
  DistributionUtils.cxx)

add_library(nuhepmc_cpputils SHARED ${IMPLEMENTATION})
target_link_libraries(nuhepmc_cpputils PUBLIC nuhepmc_options fmt::fmt)
add_library(NuHepMC::CPPUtils ALIAS nuhepmc_cpputils)
install(TARGETS nuhepmc_cpputils DESTINATION lib/)

if(NuHepMC_BUILD_STATIC_LIBS)
  add_library(nuhepmc_cpputils_static STATIC ${IMPLEMENTATION})
  target_link_libraries(nuhepmc_cpputils_static PUBLIC nuhepmc_options_static)
  add_library(NuHepMC::CPPUtils_static ALIAS nuhepmc_cpputils_static)
  install(TARGETS nuhepmc_cpputils_static DESTINATION lib/)
endif()

install(FILES ${HEADERS} DESTINATION include/NuHepMC)
