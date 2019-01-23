option (BUILD_TEAMTALK_ACE "Build customized ACE INet SSL library with SNI-enabled" ON)

if (BUILD_TEAMTALK_ACE)
  set (ACE_COMPILE_FLAGS -DENABLE_TEAMTALKACE)
endif()

if (MSVC)
  set (ACE_ROOT ${TTLIBS_ROOT}/ACE/ACE )
  set (ACE_INCLUDE_DIR ${ACE_ROOT} )
  list (APPEND ACE_INCLUDE_DIR ${ACE_ROOT}/protocols)

  set (ACE_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACEs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACEsd.lib)
  set (ACESSL_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_SSLs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_SSLsd.lib)
  set (ACEINET_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_INets.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_INetsd.lib)
  set (ACEINETSSL_STATIC_LIB optimized ${ACE_ROOT}/lib/$(PlatformName)/ACE_INet_SSLs.lib debug ${ACE_ROOT}/lib/$(PlatformName)/ACE_INet_SSLsd.lib)
  set (ACE_LINK_FLAGS ${ACEINETSSL_STATIC_LIB} ${ACESSL_STATIC_LIB} ${ACEINET_STATIC_LIB} ${ACE_STATIC_LIB})
else()

  option (ACE_STATIC "Build using static ACE libraries" ON)

  if (ACE_STATIC)
    set ( ACE_INCLUDE_DIR ${TTLIBS_ROOT}/ACE/include )
    
    set (ACE_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE.a)
    set (ACESSL_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_SSL.a)
    set (ACEINET_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_INet.a)
    set (ACEINETSSL_STATIC_LIB ${TTLIBS_ROOT}/ACE/lib/libACE_INet_SSL.a)
    set (ACE_LINK_FLAGS ${ACEINETSSL_STATIC_LIB} ${ACESSL_STATIC_LIB} ${ACEINET_STATIC_LIB} ${ACE_STATIC_LIB})
  else()
    find_library(ACE_LIBRARY ACE)
    set (ACE_LINK_FLAGS ${ACE_LIBRARY})
    find_library(ACEINET_LIBRARY ACE_INet)
    list (APPEND ACE_LINK_FLAGS ${ACEINET_LIBRARY})
    find_library(ACESSL_LIBRARY ACE_SSL)
    list (APPEND ACE_LINK_FLAGS ${ACESSL_LIBRARY})
    find_library(ACEINETSSL_LIBRARY ACE_INet_SSL)
    list (APPEND ACE_LINK_FLAGS ${ACEINETSSL_LIBRARY})
  endif()
  
endif()


if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list (APPEND ACE_COMPILE_FLAGS -pthread)
  find_library (PTHREAD_LIBRARY pthread)
  list (APPEND ACE_LINK_FLAGS ${PTHREAD_LIBRARY})
  find_library (DL_LIBRARY dl)
  list (APPEND ACE_LINK_FLAGS ${DL_LIBRARY})
endif()
