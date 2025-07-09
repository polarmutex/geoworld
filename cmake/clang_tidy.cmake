find_program(CLANGTIDY_BIN NAMES clang-tidy)

if(CLANGTIDY_BIN)
  message(STATUS "Found: clang-tidy")
  list(
        APPEND CMAKE_CXX_CLANG_TIDY
            "${CLANGTIDY_BIN}"
            "-checks=*"
    )
endif()
