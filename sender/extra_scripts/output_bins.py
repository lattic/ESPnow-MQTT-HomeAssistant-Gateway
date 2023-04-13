Import('env')
import os
import shutil

# OUTPUT_DIR = "/Users/papio/tmp/00-espnow_bin_files{}".format(os.path.sep)
# OUTPUT_DIR = "/Volumes/firmware/esp/01-with_mqtt_discovery/01-Production/0-ESPnow{}".format(os.path.sep)

OUTPUT_DIR = "/Volumes/srv/scripts/001-esp-firmware/esp/01-with_mqtt_discovery/01-Production/0-ESPnow{}".format(os.path.sep)


def _get_cpp_define_value(env, define):
    define_list = [item[-1] for item in env["CPPDEFINES"] if item[0] == define]

    if define_list:
        return define_list[0]

    return None

def _create_dirs(dirs=["firmware"]):
    # check if output directories exist and create if necessary
    if not os.path.isdir(OUTPUT_DIR):
        os.mkdir(OUTPUT_DIR)

    for d in dirs:
        if not os.path.isdir("{}{}".format(OUTPUT_DIR, d)):
            os.mkdir("{}{}".format(OUTPUT_DIR, d))

def bin_rename_copy(source, target, env):
    _create_dirs()
    variant = env["PIOENV"]
    bin_file = "{}firmware{}{}.bin".format(OUTPUT_DIR, os.path.sep, variant)
    shutil.copy(str(target[0]), bin_file)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", [bin_rename_copy])
