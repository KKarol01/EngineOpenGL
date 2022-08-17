file(GLOB_RECURSE 3dmodels "3dmodels/*.*")
file(GLOB_RECURSE shaders "shaders/*.*")
file(GLOB_RECURSE textures "textures/*.*")

foreach(item IN LISTS 3dmodels)
	string(REGEX MATCH "^.*/3dmodels/(.+$)" 3dmodel_match ${item})
	configure_file(${item} ${CMAKE_BINARY_DIR}/src/3dmodels/${CMAKE_MATCH_1} COPYONLY)
endforeach()

foreach(item IN LISTS shaders)
	get_filename_component(item_filename ${item} NAME)
	configure_file(${item} ${CMAKE_BINARY_DIR}/src/shaders/${item_filename} COPYONLY)
endforeach()

foreach(item IN LISTS textures)
	get_filename_component(item_filename ${item} NAME)
	configure_file(${item} ${CMAKE_BINARY_DIR}/src/textures/${item_filename} COPYONLY)
endforeach()