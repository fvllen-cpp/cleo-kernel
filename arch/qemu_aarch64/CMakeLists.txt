add_executable(cleo_qemu_aarch64 boot.S main.cpp)

target_link_libraries(cleo_qemu_aarch64 PRIVATE
    cleo
    cleo_warnings
    cleo_freestanding
)

target_link_options(cleo_qemu_aarch64 PRIVATE -T
    ${CMAKE_CURRENT_SOURCE_DIR}/kernel-qemu-aarch64.ld
)

set_target_properties(cleo_qemu_aarch64 PROPERTIES SUFFIX
    ".elf"
)

add_custom_target(run
    COMMAND qemu-system-aarch64 -M virt -cpu cortex-a53 -nographic
        -kernel $<TARGET_FILE:cleo_qemu_aarch64>
    DEPENDS cleo_qemu_aarch64
)
