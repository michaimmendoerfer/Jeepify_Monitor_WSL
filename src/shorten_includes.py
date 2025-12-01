import os

from platformio import fs

Import("env")

platform = env.PioPlatform()
target = env.BoardConfig().get("build.mcu", "esp32")

FRAMEWORK_SDK_DIR = fs.to_unix_path(
    os.path.join(
        platform.get_package_dir("framework-arduinoespressif32"),
        "tools",
        "sdk",
        target,
        "include",
    )
)

IS_INTEGRATION_DUMP = env.IsIntegrationDump()


def is_framework_subfolder(potential_subfolder):
    if (
        os.path.splitdrive(FRAMEWORK_SDK_DIR)[0]
        != os.path.splitdrive(potential_subfolder)[0]
    ):
        return False
    return os.path.commonpath([FRAMEWORK_SDK_DIR]) == os.path.commonpath(
        [FRAMEWORK_SDK_DIR, potential_subfolder]
    )


def shorthen_includes(env, node):
    if IS_INTEGRATION_DUMP:
        # Don't shorten include paths for IDE integrations
        return node

    includes = [fs.to_unix_path(inc) for inc in env.get("CPPPATH", [])]
    shortened_includes = []
    generic_includes = []
    for inc in includes:
        if is_framework_subfolder(inc):
            shortened_includes.append(
                "-iwithprefix/"
                + fs.to_unix_path(os.path.relpath(inc, FRAMEWORK_SDK_DIR))
            )
        else:
            generic_includes.append(inc)

    return env.Object(
        node,
        CPPPATH=generic_includes,
        CCFLAGS=env["CCFLAGS"]
        + ["-iprefix", FRAMEWORK_SDK_DIR]
        + shortened_includes,
        ASFLAGS=env["ASFLAGS"]
        + ["-iprefix", FRAMEWORK_SDK_DIR]
        + shortened_includes,
    )


env.AddBuildMiddleware(shorthen_includes)