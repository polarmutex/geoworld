find_program(CPPLINT_BIN NAMES cpplint)

if(CPPLINT_BIN)
  message(STATUS "Found: cpplint")
  message(STATUS "${CPPLINT_BIN}")
  list(
        APPEND CMAKE_CXX_CPPLINT
            "${CPPLINT_BIN}"
            "–linelength=79"
    )
endif()
