## 🏗️ Detailed Build Instructions

To build this kernel module for an Android GKI (Generic Kernel Image) device, you must follow these steps precisely to ensure the `vermagic` and `KMI` (Kernel Module Interface) match your target device.

---

### 1. Environment Setup
You need a Linux environment (Ubuntu 22.04 LTS or WSL2) and the official Android Clang toolchain.

* **Install Dependencies:**
    ```bash
    sudo apt update && sudo apt install -y build-essential bc flex bison libssl-env libelf-dev
    ```
* **Setup NDK (Clang):**
    Download NDK r27c and export the path:
    ```bash
    export PATH=$HOME/android-ndk-r27c/toolchains/llvm/prebuilt/linux-x86_64/bin:$PATH
    ```

---

### 2. Prepare Kernel Source
The module must be compiled against the **Android Common Kernel (ACK)** source that matches your device's version.

1.  **Clone the Kernel Source:**
    ```bash
    mkdir android_kernel && cd android_kernel
    git clone [https://android.googlesource.com/kernel/common](https://android.googlesource.com/kernel/common) -b android12-5.10
    cd common
    ```

2.  **Checkout Specific Version:**
    List versions and switch to your target (e.g., v5.10.223):
    ```bash
    git tag -l "v5.10.*"
    git checkout v5.10.223
    ```

3.  **Handle Version Suffix (`-dirty` or `-android`):**
    Open the `Makefile` in the kernel root and modify the `EXTRAVERSION` if your phone's `uname -r` shows a specific suffix (e.g., `-android12-9`):
    ```bash
    nano Makefile
    # Edit line: EXTRAVERSION = -android12-9
    ```



---

### 3. Initialize Kernel Configuration
Before compiling the module, the kernel headers and configuration must be prepared.

```bash
# Generate the GKI default config
make ARCH=arm64 LLVM=1 LLVM_IAS=1 CROSS_COMPILE=aarch64-linux-android- gki_defconfig

# Prepare symbols and headers
make ARCH=arm64 LLVM=1 LLVM_IAS=1 CROSS_COMPILE=aarch64-linux-android- modules_prepare
