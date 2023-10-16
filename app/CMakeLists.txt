add_executable(NuHepMCXSTool NuHepMCXSTool.cxx)

target_link_libraries(NuHepMCXSTool PRIVATE NuHepMC::CPPUtils)

<<<<<<< Updated upstream
install(TARGETS xstool DESTINATION bin)

add_executable(testdist testdist.cxx)

target_link_libraries(testdist PRIVATE NuHepMC::CPPUtils)

install(TARGETS testdist DESTINATION bin)
=======
CPMAddPackage(
    NAME fmt
    GIT_TAG 8.1.1
    GITHUB_REPOSITORY fmtlib/fmt
)

add_executable(NuHepMC2dot NuHepMC2dot.cxx)

target_link_libraries(NuHepMC2dot PRIVATE NuHepMC::CPPUtils fmt::fmt)


install(TARGETS NuHepMCXSTool NuHepMC2dot DESTINATION bin)
>>>>>>> Stashed changes
