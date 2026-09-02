# For more information about build system see
# https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/build-system.html
# The following five lines of boilerplate have to be in your project's
# CMakeLists in this exact order for cmake to work correctly
cmake_minimum_required(VERSION 3.5)
include($ENV{IDF_PATH}/tools/cmake/project.cmake)
add_compile_options("-Wno-format")
# Baseline app version (M6 OTA): esp_app_get_description()->version, compared
# against GitHub Releases' tag_name by dial_ota. Bump per release; the tag
# pushed to GitHub ("somnus-vX.Y.Z") must match this for the update check to
# ever consider a release "current" once installed.
#
# Reset to 0.1.0 on 2026-09-01 (docs/SPEC-ota-readiness.md §7): this project
# forked from chris023/orion-waveshare-rotary-dial, whose history (tags
# "dial-v1.0.0"-"dial-v1.4.2") is real lineage but not a Somnus release --
# nothing has ever shipped under the Somnus name. The prior value here
# ("1.4.2") was inherited verbatim from that fork and never bumped down,
# which would have made every device silently reject any 0.x release as a
# downgrade (is_newer() has no concept of a renumbering, only "older").
# 1.0.0 is reserved for when docs/V1-scope.md is actually complete -- see
# CHANGELOG.md's Somnus section header.
set(PROJECT_VER "0.1.0")
project(somnus-dial)
