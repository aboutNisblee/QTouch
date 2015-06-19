# Copyright (c) 2014, Moritz Nisblé (moritz.nisble@gmx.de)
# All rights reserved.
#
# Melpers - Moritz' little helpers ;)
# Macros for adding sources and include directories.
#
# Description will follow...
#
# NOTE: It is crucial not to break the chain of the source variable.
#       The variable is passed upwards via PARENT_SCOPE. So take care not to break the chain
#       when adding CMakeLists.txt for directories that contain no source files.
#       In that case you may need to pass the variable upwards by hand using
#       'set(SRCS ${SRCS} PARENT_SCOPE)' (e.g. for a source variable named SRCS)
#
# Changelog:
#            v1.0: Basic versions of add_sources, add_dirs, print_sources, print_includes
#            v1.1: Replaced print_includes by print_include_property_per_subdir
#                  Added print_processed_subdirs
#            v1.2: Added addprefix and addsuffix functions
#            v1.3: Added print_list function; Renamed prefix/suffix functions
#                  Declared add_dirs macro (recursive add_subdirectories) as deprecated.

# Add source macro
macro(add_sources SOURCE_VAR)
	set(_relPath)

	# Get the realtive path from root to macro calling file
    file(RELATIVE_PATH _relPathToMacro "${CMAKE_SOURCE_DIR}" "${CMAKE_CURRENT_SOURCE_DIR}")
# 	message(STATUS "[MELPERS] _relPathToMacro: ${_relPathToMacro}")

	if(_relPathToMacro)
		set(_relPath "${_relPathToMacro}/")
	endif()

    foreach(_src ${ARGN})
# 		message(STATUS "[MELPERS] _src: ${_src}")

		# Check if source is passed absolute
		if(IS_ABSOLUTE "${_src}")
			file(RELATIVE_PATH _relPathToSource "${CMAKE_CURRENT_SOURCE_DIR}" "${_src}")
# 			message(STATUS "[MELPERS] _relPathToSource: ${_relPathToSource}")
			list(APPEND ${SOURCE_VAR} "${_relPath}${_relPathToSource}")
		else()
			list(APPEND ${SOURCE_VAR} "${_relPath}${_src}")
		endif()

    endforeach()

	# If invoked from subdirectory, propagate variable into parent scope
    if(_relPathToMacro)
        set(${SOURCE_VAR} ${${SOURCE_VAR}} PARENT_SCOPE)
    endif()
# 	message(STATUS "[MELPERS] SOURCE_VAR: ${${SOURCE_VAR}}")
endmacro()

function(print_list text list)
    message(STATUS "********************************************************************************")
    message(STATUS "${text}")
    foreach(item ${${list}})
        message(STATUS "    ${item}")
    endforeach()
    message(STATUS "********************************************************************************")
endfunction()

function(add_prefix list prefix)
	foreach(l ${${list}})
		list(APPEND _list ${prefix}${l})
	endforeach()
	set(${list} ${_list} PARENT_SCOPE)
endfunction()

function(add_suffix list suffix)
	foreach(l ${${list}})
		list(APPEND _list ${l}${suffix})
	endforeach()
	set(${list} ${_list} PARENT_SCOPE)
endfunction()
