file(GLOB cpp_toolkits_list ${CppToolKits_Src}/*.cpp ${CppToolKits_Src}/*.h
        ${CppToolKits_Src}/*/*.h ${CppToolKits_Src}/*/*.cpp
        ${CppToolKits_Src}/*/*/*.h ${CppToolKits_Src}/*/*/*.cpp)

message(${cpp_toolkits_list})
add_library(cpptoolkits STATIC ${cpp_toolkits_list})
target_link_libraries(cpptoolkits ${LINK_LIB_LIST})