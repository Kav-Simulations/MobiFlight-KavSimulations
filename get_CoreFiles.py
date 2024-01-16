import os
import subprocess

Import("env")

CORESOURCE_DIR = env.subst("$PROJECT_DIR/src")
CORESOURCE_TAG = env.GetProjectOption("custom_core_firmware_version")
print("Compiling for Core Version: " + CORESOURCE_TAG)
    
def run_command(command):
    # Break the command into parts using a list
    process = subprocess.Popen(command, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout, stderr = process.communicate()
    if process.returncode != 0:
        print(f"Error executing command: {stderr.decode()}")
        env.Exit(1)
    else:
        print(stdout.decode())

if not os.path.exists(CORESOURCE_DIR):
    print("Cloning 'core src' from Mobiflight-Firmware repo ...")
    run_command([
        "git", "clone", "--depth", "1",
        "--filter=blob:none", "--sparse",
        "--branch", f"{CORESOURCE_TAG}",
        "https://github.com/MobiFlight/MobiFlight-FirmwareSource",
        f"{CORESOURCE_DIR}"
    ])
    run_command([
        "git", f"--work-tree={CORESOURCE_DIR}", 
        f"--git-dir={CORESOURCE_DIR}/.git", 
        "sparse-checkout", 
        "set", "_Boards", "src"
    ])
    if os.path.isfile(CORESOURCE_DIR + "/platformio.ini"):
        os.remove(CORESOURCE_DIR + "/platformio.ini")
else:
    print("Checking for 'core src' MobiFlight-Firmware repo updates ...")
    run_command([
        "git", f"--work-tree={CORESOURCE_DIR}",
        f"--git-dir={CORESOURCE_DIR}/.git",
        "pull", "origin", CORESOURCE_TAG,
        "--depth", "100"
    ])