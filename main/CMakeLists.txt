# ============================================================================
#  The `main` component.
#
#  Every ESP-IDF project has a `main` component — it's the one the framework
#  calls into (your `app_main`). A "component" is IDF's unit of reusable code:
#  a folder with sources, headers, and this registration call. As the project
#  grows (display driver, touch, etc.) each becomes its own component with its
#  own CMakeLists.txt.
#
#  SRCS         = the source files to compile in this component.
#  INCLUDE_DIRS = folders added to the include path for anyone using it
#                 ( "." = this folder, so headers here are findable ).
# ============================================================================
idf_component_register(SRCS "main.c"
                       INCLUDE_DIRS ".")
