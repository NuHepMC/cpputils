pybind11_add_module(pyNuHepMC SHARED pyNuHepMC.cxx)

target_link_libraries(pyNuHepMC PRIVATE NuHepMC::CPPUtils pybind11::module)
target_include_directories(pyNuHepMC PRIVATE ${Python3_INCLUDE_DIRS})

install(TARGETS pyNuHepMC
    LIBRARY DESTINATION ${NuHepMC_CPPUtils_PYTHONPATH})