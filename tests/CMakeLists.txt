add_executable(ReaderTests ReaderTests.cxx)
target_link_libraries(ReaderTests PRIVATE Catch2::Catch2WithMain NuHepMC::CPPUtils)
target_include_directories(ReaderTests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(ReaderTests)

add_executable(AttributeTests AttributeTests.cxx)
target_link_libraries(AttributeTests PRIVATE Catch2::Catch2WithMain NuHepMC::CPPUtils)
target_include_directories(AttributeTests PRIVATE $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}../>)

catch_discover_tests(AttributeTests)