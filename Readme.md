# 🛰️ Android Physical Memory Read/Write Kernel Module (GKI)

This project is a high-performance Linux kernel module (`.ko`) designed for Android devices running **GKI (Generic Kernel Image)**. It provides a specialized interface to perform direct physical memory operations, bypassing standard virtual memory restrictions for debugging, research, and memory analysis purposes.

---

### ⚙️ How It Works
The module creates a character device (or utilizes an IOCTL interface) to bridge the gap between user-space and the system's physical RAM.

1. **Memory Mapping**: Since the CPU uses a Virtual Memory Unit (MMU), the module uses kernel functions like `kmap_atomic` or `vmap` to map specific physical addresses into the kernel's virtual address space temporarily.
2. **Kernel-to-User Data Transfer**: It utilizes `copy_to_user` and `copy_from_user` to safely move data between the restricted kernel memory space and your user-space application.
3. **GKI Compliance**: The module is built against the **Android Common Kernel (ACK)**, ensuring compatibility with the strict Kernel Module Interface (KMI) introduced in Android 12 (Kernel 5.10+).



---

### 🛠️ Technical Specifications
* **Target Architecture**: ARM64 (aarch64)
* **Kernel Base**: Linux 5.10.x / 6.1.x (Android GKI)
* **Toolchain**: LLVM/Clang (as per Google's official build requirements)

#### ✨ Key Features
* **Direct Access**: Read/Write physical memory bypassing `PAGEMAP`.
* **Efficiency**: Support for Large Pages and specialized memory offsets.
* **Performance**: Low-latency execution for real-time memory monitoring.

---

### 🏗️ Building the Module

Follow these steps to compile the module for your specific GKI kernel version.

#### 1. Requirements
* **Android NDK**: Version `r27c` or newer.
* **Kernel Source**: Version matching your target device (e.g., `5.10.223`).
* **Environment**: Linux (Ubuntu 22.04+ or WSL2).

#### 2. Compilation
Set your NDK toolchain path and run the build command from your project directory:

```bash
# Set NDK Toolchain Path
export PATH=$HOME/android-ndk-r27c/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH

# Compile
make -C /path/to/kernel/common M=$PWD \
    ARCH=arm64 \
    LLVM=1 LLVM_IAS=1 \
    CROSS_COMPILE=aarch64-linux-android- \
    modules
