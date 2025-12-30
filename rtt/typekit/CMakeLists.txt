IF (PLUGINS_ENABLE_TYPEKIT)
  if (NOT PLUGINS_STD_TYPES_SUPPORT)
    ADD_DEFINITIONS( -DRTT_NO_STD_TYPES )
  endif (NOT PLUGINS_STD_TYPES_SUPPORT)

  set(CPPS
     RealTimeTypekit.cpp
     RealTimeTypekitStdTypes.cpp
     RealTimeTypekitTypes.cpp
  )


  set(HPPS
    BoolTypeInfo.hpp
    RealTimeTypekit.hpp
    RTStringTypeInfo.hpp
    StdStringTypeInfo.hpp
    StdTypeInfo.hpp
    Types.hpp
  )

  if (PLUGINS_ENABLE_SCRIPTING)
    list(APPEND CPPS
       RealTimeTypekitConstructors.cpp
       RealTimeTypekitGlobals.cpp
       RealTimeTypekitOperators.cpp
       RealTimeTypekitTypes2.cpp
    )

    list (APPEND HPPS
      RTTTypes.hpp
      ConnPolicyType.hpp
      StdVectorTypeInfo.hpp
    )
  endif()

  GLOBAL_ADD_INCLUDE( rtt/typekit ${HPPS})
  GLOBAL_ADD_INCLUDE( rtt/typekit ${CMAKE_CURRENT_BINARY_DIR}/rtt-typekit-config.h)

  # Due to generation of some .h files in build directories, we also need to include some build dirs in our include paths.
  INCLUDE_DIRECTORIES(BEFORE ${PROJ_SOURCE_DIR} ${PROJ_SOURCE_DIR}/rtt ${PROJ_SOURCE_DIR}/rtt/os ${PROJ_SOURCE_DIR}/rtt/os/${OROCOS_TARGET} )
  INCLUDE_DIRECTORIES(BEFORE ${PROJ_BINARY_DIR}/rtt ${PROJ_BINARY_DIR}/rtt/os ${PROJ_BINARY_DIR}/rtt/os/${OROCOS_TARGET} )
  INCLUDE_DIRECTORIES(BEFORE ${PROJ_BINARY_DIR}/rtt/typekit )
  
  #SET_SOURCE_FILES_PROPERTIES( ${CPPS} PROPERTIES COMPILE_FLAGS "-fvisibility=hidden")    

  SET(RTT_DEFINITIONS "${OROCOS-RTT_DEFINITIONS}")
  set(DLL_EXPORT_PREFIX typekit)
  ADD_RTT_TYPEKIT(rtt-typekit ${RTT_VERSION} ${CPPS})

ENDIF (PLUGINS_ENABLE_TYPEKIT)
