add_library(HeliumType
        argv.cc
        composite_type.cc
        corner.cc
        input_spec.cc
        io_helper.cc
        primitive_type.cc
        sequential_type.cc
        type.cc
        type_helper.cc
        variable.cc
        )
target_link_libraries(HeliumType HeliumUtils)
