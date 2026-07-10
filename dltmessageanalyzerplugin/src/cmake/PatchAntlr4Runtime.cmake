if(NOT DEFINED ANTLR4_ROOT)
  message(FATAL_ERROR "ANTLR4_ROOT is not defined")
endif()

set(PROFILING_ATN_SIMULATOR "${ANTLR4_ROOT}/runtime/Cpp/runtime/src/atn/ProfilingATNSimulator.cpp")

if(NOT EXISTS "${PROFILING_ATN_SIMULATOR}")
  message(FATAL_ERROR "ANTLR4 runtime source file is missing: ${PROFILING_ATN_SIMULATOR}")
endif()

file(READ "${PROFILING_ATN_SIMULATOR}" PROFILING_ATN_SIMULATOR_CONTENT)

if(NOT PROFILING_ATN_SIMULATOR_CONTENT MATCHES "#include <chrono>")
  string(REPLACE
    "#include \"atn/PredicateEvalInfo.h\""
    "#include <chrono>\n\n#include \"atn/PredicateEvalInfo.h\""
    PROFILING_ATN_SIMULATOR_CONTENT
    "${PROFILING_ATN_SIMULATOR_CONTENT}")
endif()

if(NOT PROFILING_ATN_SIMULATOR_CONTENT MATCHES "using namespace std::chrono;")
  string(REPLACE
    "#include \"atn/PredicateEvalInfo.h\""
    "#include \"atn/PredicateEvalInfo.h\"\n\nusing namespace std::chrono;"
    PROFILING_ATN_SIMULATOR_CONTENT
    "${PROFILING_ATN_SIMULATOR_CONTENT}")
endif()

file(WRITE "${PROFILING_ATN_SIMULATOR}" "${PROFILING_ATN_SIMULATOR_CONTENT}")
