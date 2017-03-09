add_library(HeliumWorkflow
        analyzer.cc
        builder.cc
        formula.cc
        generator.cc
        helium.cc
        segment.cc
        tester.cc)
target_link_libraries(HeliumWorkflow HeliumUtils HeliumType)
