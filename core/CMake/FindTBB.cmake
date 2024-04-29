find_package(PkgConfig REQUIRED)
pkg_search_module(TBB tbb)

set( TBB_LIB )
if( TBB_FOUND )
    set( TBB_LIB ${TBB_LIBRARIES} )
    message( STATUS ">> TBB found.")
else()
    message( WARNING ">> TBB could not be found.")
    set( TBB_FOUND False )
endif()