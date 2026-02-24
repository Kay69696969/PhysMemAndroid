#  Android Physical Memory Read/Write Kernel Module (GKI)

This project is a high-performance Linux kernel module (`.ko`) designed for Android devices running **GKI (Generic Kernel Image)**. It provides a specialized interface to perform direct physical memory operations, bypassing standard virtual memory restrictions for debugging, research, and memory analysis purposes.

---
## Tested in games: Call Of Duty Mobile, Free Fire, Standoff2

###  How It Works

The module creates a communication bridge using the Generic Netlink protocol to handle requests between user-space and the system's physical RAM.

1. **Memory Mapping**: Since the CPU uses a Virtual Memory Unit (MMU), the module uses `kmap_atomic` to map specific physical addresses into the kernel's virtual address space temporarily.
2. **Address Translation**: It implements a manual **Page Table Walk** (PGD -> P4D -> PUD -> PMD -> PTE) to resolve a process's virtual address to a raw physical address.

3. **GKI Compliance**: Built against the **Android Common Kernel (ACK)**, ensuring compatibility with the strict Kernel Module Interface (KMI) introduced in Android 12 (Kernel 5.10+).

---

####  Implementation Details

* **Communication**: Uses `Generic Netlink` (Family: `MY_GENL_BUS`) for efficient asynchronous data transfer.
* **Atomicity**: Uses `kmap_atomic` for high-speed, interrupt-safe mapping of physical pages.
* **Access Control**: Supports both `READ_PHYS` and `WRITE_PHYS` operations via IOCTL-like commands over Netlink.

###  Technical Specifications

* **Target Architecture**: ARM64 (aarch64)
* **Kernel Base**: Linux 5.10.x / 6.1.x (Android GKI)
* **Toolchain**: LLVM/Clang (Official Google Toolchain as per GKI requirements)

#### Key Features

* **Direct Access**: Read/Write physical memory bypassing `PAGEMAP`.
* **Efficiency**: Support for Large Pages and specialized memory offsets.
* **Performance**: Low-latency execution for real-time memory monitoring.

---

##  Detailed Build Instructions

To build this kernel module, you must precisely follow these steps to ensure the `vermagic` and `KMI` match your target device.

### 1. Environment & Path Setup

You need a Linux environment (Ubuntu 22.04 LTS or WSL2). First, install the necessary build tools:

```bash
sudo apt update && sudo apt install -y build-essential bc flex bison libssl-dev libelf-dev
```
## Setup NDK (Clang):
Download **NDK r27c** (or the version matching your project) and add the toolchain to your system's PATH. This is required for the `make` command to locate the Clang compiler and associated tools.

```bash
# Replace with your actual NDK extraction path
export PATH=$HOME/android-ndk-r27c/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
```

## Prepare Kernel Source:
The module must be compiled against the **Android Common Kernel (ACK)** source that matches your device's version exactly.

```bash
mkdir android_kernel && cd android_kernel
git clone [https://android.googlesource.com/kernel/common](https://android.googlesource.com/kernel/common) -b android12-5.10
cd common
# Check your phone's 'uname -r' and checkout the matching tag
git checkout v5.10.223
```

## Handle Version Suffix:
Check `adb shell uname -r` on your device. If it contains a suffix like `-android12-9-g...`, you must edit the kernel `Makefile` to match it.

```bash
# Locate EXTRAVERSION at the top and set it accordingly:
# EXTRAVERSION = -android12-9
nano Makefile
```
## Initialize Configuration:
Before building the module, you must prepare the kernel headers and configuration:
```bash
# 1. Load GKI default configuration
make ARCH=arm64 LLVM=1 LLVM_IAS=1 CROSS_COMPILE=aarch64-linux-android- gki_defconfig

# 2. Prepare symbols and headers for module compilation
make ARCH=arm64 LLVM=1 LLVM_IAS=1 CROSS_COMPILE=aarch64-linux-android- modules_prepare
```

## Build the Module:
```bash
make -C ~/android_kernel/common M=$PWD \
    ARCH=arm64 \
    LLVM=1 LLVM_IAS=1 \
    CLANG_TRIPLE=aarch64-linux-gnu- \
    CROSS_COMPILE=aarch64-linux-android- \
    modules
```
## Command Parameters Breakdown:
```bash
-C [path]: Tells make to change to the kernel source directory to use its root Makefile.

M=$PWD: Informs the build system that the external module is located in the current directory.

ARCH=arm64: Sets the target architecture to 64-bit ARM.

LLVM=1 LLVM_IAS=1: Forces the use of Clang and the Integrated Assembler (required for GKI).

CLANG_TRIPLE: Specifies the target triple for the Clang compiler.

CROSS_COMPILE: Points to the Android-specific cross-compiler tool prefix.
```
## SOME IN GAME PICS
<img width="1280" height="591" alt="image" src="https://github.com/user-attachments/assets/5e2995e9-7ee9-45fe-bb01-0c6989b0c1cf" />

<img width="1280" height="591" alt="image" src="https://github.com/user-attachments/assets/d5af34ee-aaa0-4673-ab50-680bb4b32f93" />



## FOR UPDATES JOIN UP : t.me/villagehooknew
<img width="1024" height="1024" alt="image" src="https://github.com/user-attachments/assets/78a92b28-d40b-4dbb-bb53-7cc9d0f21f90" />

## DEVELOPER : Paranoia(me)
