function(get_major_version full_version major_version)
	if (WIN32)
		set(MSVC_80_VERSION 8)
		set(MSVC_90_VERSION 9)
		set(MSVC_100_VERSION 10)
		set(MSVC_110_VERSION 11)
		set(MSVC_120_VERSION 12)
		set(MSVC_140_VERSION 14)
		set(MSVC_141_VERSION 15)
		set(MSVC_142_VERSION 16)
		set(majver "${MSVC_${MSVC_TOOLSET_VERSION}_VERSION}")
	else()
		string(REPLACE "." ";" verlist ${full_version})
		list(GET verlist 0 majver)
	endif()
	set(compiler_version ${majver} PARENT_SCOPE)
endfunction()


macro(get_settings)

	set(compiler_name UNSUPPORTED)
	if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
		set(compiler_name gcc)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
		set(compiler_name apple-clang)
	elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "MSVC")
		set(compiler_name "Visual Studio")
	endif()

	set(compiler_version UNSUPPORTED)
	get_major_version("${CMAKE_CXX_COMPILER_VERSION}" "${compiler_version}")

	if ("${CMAKE_BUILD_TYPE}" STREQUAL "")
		set(build_type "Release")
	else()
		set(build_type ${CMAKE_BUILD_TYPE})
	endif()

	set(os_name UNSUPPORTED)
	if (APPLE)
		set(os_name Macos)
	elseif (UNIX AND NOT APPLE)
		set(os_name Linux)
	elseif (WIN32)
		set(os_name Windows)
	endif()


endmacro(get_settings)

# Artifactory requires the following information to find a package
# The file conaninfo.txt is used for the search as it is saved
# with the conan properties visible to the AQL query
#
# conan.settings.compiler.version windows:15 linux:9 clang:10
# conan.settings.compiler  - gcc, Visual Studio, apple-clang
# conan.settings.build_type Release Debug
# conan.settings.os Linux, Windows, Macos
# conan.package.version latest or 0.1.0 or 0.2.0, or 0.3.0
# conan.package.name
# conan.package.user     lkeb
# conan.package.channel  stable
# filename conaninfo.txt
#

macro(get_artifactory_package 
		package_name package_version package_builder 
		compiler_name compiler_version os_name build_type )
	configure_file(${CMAKE_MODULE_PATH}/aql.json.in aql.json)
	set(CURL_COMMAND)
	list(APPEND CURL_COMMAND curl -k -u conan-user:XQlM?4KxtCPOp@0t -X POST -H "content-type: text/plain" --data @aql.json https://lkeb-artifactory.lumc.nl:443/artifactory/api/search/aql)
	execute_process(COMMAND ${CURL_COMMAND}	RESULT_VARIABLE results OUTPUT_FILE ${CMAKE_SOURCE_DIR}/aql_out.txt)

	set(path_match "\"path\"")
	file(STRINGS ${CMAKE_SOURCE_DIR}/aql_out.txt result_file REGEX "${path_match}")
	#message("result_file ${result_file}")
	list(LENGTH result_file res_length)

	message("result length ${res_length}")
	foreach(line ${result_file})
		message("package found: ${line}\n")
	endforeach()

	if (res_length LESS 1)
		message(FATAL_ERROR "No matching packages found. Please check the query.")
	endif()

	if (res_length GREATER 1)
		message(FATAL_ERROR "Too many matching packages found. Contact the HDPS group for more info and supply the aql_out.txt and aql.json files")
	endif()

	list(GET result_file 0 path_line)
	#message("Retrieved path ${path_line}")
	string(REGEX MATCH "[^ \"]*/0" package_id "${path_line}")
	#message("package id ${package_id}")
	set(package_url "https://lkeb-artifactory.lumc.nl/artifactory/conan/${package_id}/conan_package.tgz")
	message("package url ${package_url} - name ${package_name}")
	file(DOWNLOAD ${package_url} "${CMAKE_SOURCE_DIR}/${package_name}.tgz")

	execute_process(COMMAND cmake -E make_directory "${CMAKE_SOURCE_DIR}/${package_name}")
	execute_process(COMMAND cmake -E tar xvf "${CMAKE_SOURCE_DIR}/${package_name}.tgz" WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}/${package_name}")

endmacro()

# Install a package version from the artifactory.
# The specific version will be based on the current OS, compiler
# and build settings
function(install_artifactory_package package_name package_version package_builder)
	message("Installing package * ${package_name} * from lkeb-artifactory.lumc.nl")
    get_settings()
    set(package_name ${package_name})
    set(package_version ${package_version})
    set(package_builder ${package_builder})

	get_artifactory_package("${package_name}" "${package_version}" "${package_builder}"
		"${compiler_name}" "${compiler_version}"
		"${os_name}" "${build_type}")
    # add the HDILib to the module path
    set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/${package_name}" ${CMAKE_MODULE_PATH} PARENT_SCOPE)
endfunction()