#https://cmake.org/cmake/help/v3.10/manual/cmake-buildsystem.7.html
cmake_minimum_required(VERSION 3.10)
project(goose)

# bin and libs
set(BIN_NAMES goose)
set(LIB_NAMES GooseSub DrvLogger helloc)

### engine ###
# project struct
# 	bin_name
#		src
#			*.c
#			*.h
#		include
#			bin_name.h
#		prereq
# 	lib_name
#		src
#			*.c
#			*.h
#		include
#			lib_name.h
#		prereq
#	bin_nameprereq
#	lib_nameprereq
# prereq-s
#	set(PREREQ_H_objname other_obj_names) 	# includes from obj/include
#	set(PREREQ_objname other_obj_names)		# libother_obj.so
#	set(LDFLAGS_objname thpt_libs) 			# libthirdparty.so

set(BUILDING BIN_NAMES LIB_NAMES)
foreach(cl IN LISTS BUILDING)
	foreach(obj IN LISTS ${cl})
		FILE(GLOB prereqs1 ${obj}/prereq*)
		FILE(GLOB prereqs2 ${obj}prereq)
		FILE(GLOB src ${obj}/src/*)
		FILE(GLOB inc ${obj}/include/*)
		foreach(finc IN LISTS prereqs1 prereqs2)
			include(${finc})
		endforeach()
		foreach(prereqdir IN LISTS PREREQ_H_${obj})
			FILE(GLOB preinc ${prereqdir}/include/*)
			include_directories(${prereqdir}/include)
			list(APPEND inc2 ${preinc})
		endforeach()
		include_directories(${obj}/include)
		if (${obj} IN_LIST BIN_NAMES)
			add_executable(${obj} ${src} ${inc} ${inc2})
		endif()
		if (${obj} IN_LIST LIB_NAMES)
			add_library(${obj} SHARED ${src} ${inc} ${inc2})
		endif()
		foreach(ldfl IN LISTS LDFLAGS_${obj})
			find_library(${ldfl}_LIBRARY ${ldfl})
			list(APPEND ${obj}_ldfl_list ${ldfl})
		endforeach()
	endforeach()
	foreach(obj IN LISTS ${cl})
		target_link_libraries(${obj} ${${obj}_ldfl_list} ${PREREQ_${obj}} )
	endforeach()
endforeach()

