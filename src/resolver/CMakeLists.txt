add_library(HeliumResolver
        global_variable.cc
        header_resolver.cc
        resolver.cc
        snippet.cc
        snippet_db.cc
        snippet_registry.cc
        system_resolver.cc)
target_link_libraries(HeliumResolver HeliumUtils HeliumParser)
