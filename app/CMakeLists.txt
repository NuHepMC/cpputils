add_executable(xstool xstool.cxx)

target_link_libraries(xstool PRIVATE NuHepMC::CPPUtils)

install(TARGETS xstool DESTINATION bin)

add_executable(testdist testdist.cxx)

target_link_libraries(testdist PRIVATE NuHepMC::CPPUtils)

install(TARGETS testdist DESTINATION bin)
