cmake_minimum_required(VERSION 3.10)

# Nombre del proyecto
project(DecodificadorPRT7 VERSION 1.0 LANGUAGES CXX)

# Forzar el uso de MinGW si está disponible en Windows
if(WIN32 AND NOT MSVC)
    set(CMAKE_GENERATOR "MinGW Makefiles" CACHE INTERNAL "" FORCE)
endif()

# Establecer el estándar de C++
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Configuración de compilación
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Opciones de compilación
set(CMAKE_CXX_FLAGS_DEBUG "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -Wall")

# Archivos fuente
set(SOURCES
    main.cpp
)

# Crear el ejecutable
add_executable(decodificador_prt7 ${SOURCES})

# Configuración específica para Windows
if(WIN32)
    target_compile_definitions(decodificador_prt7 PRIVATE _WIN32)
    message(STATUS "Configurando para Windows")
endif()

# Configuración específica para Linux
if(UNIX)
    message(STATUS "Configurando para Linux/Unix")
endif()

# Información de compilación
message(STATUS "Tipo de compilación: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compilador C++: ${CMAKE_CXX_COMPILER}")
message(STATUS "Flags de compilación: ${CMAKE_CXX_FLAGS}")

# Instalación
install(TARGETS decodificador_prt7
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib)

# Crear paquete
set(CPACK_PACKAGE_NAME "DecodificadorPRT7")
set(CPACK_PACKAGE_VERSION "1.0.0")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Decodificador de Protocolo Industrial PRT-7")
set(CPACK_PACKAGE_VENDOR "Universidad")
set(CPACK_PACKAGE_CONTACT "tu_email@ejemplo.com")

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)