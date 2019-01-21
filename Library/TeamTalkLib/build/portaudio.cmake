include (ttlib)

if (MSVC)
  set (PORTAUDIO_INCLUDE_DIR ${TTLIBS_ROOT}/portaudio/include)

  set (PORTAUDIO_LINK_FLAGS optimized ${TTLIBS_ROOT}/portaudio/lib/$(PlatformName)/pastatic.lib
    debug ${TTLIBS_ROOT}/portaudio/lib/$(PlatformName)/pastaticd.lib)

else()

  option (PORTAUDIO_STATIC "Build portaudio using static libraries" ON)

  if (PORTAUDIO_STATIC)
    set (PORTAUDIO_INCLUDE_DIR ${TTLIBS_ROOT}/portaudio/include)
    set (PORTAUDIO_LINK_FLAGS ${TTLIBS_ROOT}/portaudio/lib/libportaudio.a)
  else()
    # Ubuntu: portaudio19-dev
    find_library(PORTAUDIO_LIBRARY portaudio)
    set (PORTAUDIO_LINK_FLAGS ${PORTAUDIO_LIBRARY})
  endif()
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin" )
  find_library (CARBON_LIBRARY Carbon)
  set (PORTAUDIO_LINK_FLAGS ${CARBON_LIBRARY})
  find_library (AUDIOUNIT_LIBRARY AudioUnit)
  list (APPEND PORTAUDIO_LINK_FLAGS ${AUDIOUNIT_LIBRARY} )
  find_library (COREAUDIO_LIBRARY CoreAudio)
  list (APPEND PORTAUDIO_LINK_FLAGS ${COREAUDIO_LIBRARY} )
  find_library (AUDIOTOOLBOX_LIBRARY AudioToolBox )
  list (APPEND PORTAUDIO_LINK_FLAGS ${AUDIOTOOLBOX_LIBRARY} )
endif()

if ( ${CMAKE_SYSTEM_NAME} MATCHES "Linux" )
  # Ubuntu: libasound2-dev
  find_library(ASOUND_LIBRARY asound)
  list (APPEND PORTAUDIO_LINK_FLAGS ${ASOUND_LIBRARY})
endif()
