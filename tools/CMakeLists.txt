project(AITOOLS_TOOLS)

add_executable(learnrf learnrf.cpp)
target_link_libraries(learnrf LINK_PUBLIC aitoolslib ${TBB_IMPORTED_TARGETS})
if(TBB_FOUND)
    target_link_libraries(learnrf PUBLIC TBB::tbb)
endif()

add_executable(buildgef buildgef.cpp)
target_link_libraries(buildgef LINK_PUBLIC aitoolslib)

add_executable(samplepc samplepc.cpp)
target_link_libraries(samplepc LINK_PUBLIC aitoolslib)

add_executable(datasetinfo datasetinfo.cpp)
target_link_libraries(datasetinfo LINK_PUBLIC aitoolslib)

add_executable(learndt learndt.cpp)
target_link_libraries(learndt LINK_PUBLIC aitoolslib)

add_executable(makedataset makedataset.cpp)
target_link_libraries(makedataset LINK_PUBLIC aitoolslib)
if(TBB_FOUND)
    target_link_libraries(makedataset PUBLIC TBB::tbb)
endif()

add_executable(pc pc.cpp)
target_link_libraries(pc LINK_PUBLIC aitoolslib)

install(TARGETS learnrf buildgef samplepc datasetinfo learndt makedataset RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}")
