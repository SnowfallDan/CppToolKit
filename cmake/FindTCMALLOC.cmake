#- Find libtcmalloc

# linux系统下调用pkg-config查找tcmalloc
if (NOT WIN32)
    include(FindPkgConfig)
    unset(_verexp)
    if (TCMALLOC_FIND_VERSION)
        if (TCMALLOC_FIND_VERSION_EXACT)
            set(_verexp "=${TCMALLOC_FIND_VERSION}")
        else ()
            set(_verexp ">=${TCMALLOC_FIND_VERSION}")
        endif ()
    endif ()
    pkg_check_modules(TCMALLOC libtcmalloc_minimal${_verexp})
endif ()

# handle the QUIETLY and REQUIRED arguments and set TCMALLOC_FOUND to TRUE if
# all listed variables are TRUE
include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(TCMALLOC DEFAULT_MSG TCMALLOC_LIBRARIES TCMALLOC_INCLUDE_DIRS)
