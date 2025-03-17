#imguizmo
string(TIMESTAMP BEFORE "%s")

CPMAddPackage(
	NAME "imguizmo"
	URL "https://github.com/CedricGuillemet/ImGuizmo/archive/refs/tags/1.83.zip"
)
find_package(imguizmo REQUIRED)
set (imguizmo_INCLUDE_DIR ${imguizmo_SOURCE_DIR}/include)
include_directories(${imguizmo_INCLUDE_DIR})
string(TIMESTAMP AFTER "%s")
math(EXPR DELTAimguizmo "${AFTER}-${BEFORE}")
MESSAGE(STATUS "imguizmo TIME: ${DELTAimguizmo}s")
