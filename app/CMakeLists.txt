add_executable(xstool xstool.cxx)

target_link_libraries(xstool PRIVATE NuHepMC::CPPUtils)

install(TARGETS xstool DESTINATION bin)
