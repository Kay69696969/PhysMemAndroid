# 🛰️ Android Physical Memory Read/Write Kernel Module (GKI)

This project is a high-performance Linux kernel module (`.ko`) designed for Android devices running **GKI (Generic Kernel Image)**. It provides a specialized interface to perform direct physical memory operations, bypassing standard virtual memory restrictions for debugging and memory analysis purposes.

---

### ⚙️ How It Works
The module creates a communication bridge using the Generic Netlink protocol to handle requests between user-space and the system's physical RAM.

1. **Memory Mapping**: Since the CPU uses a Virtual Memory Unit (MMU), the module uses `kmap_atomic` to map specific physical addresses into the kernel's virtual address space temporarily.
2. **Address Translation**: It implements a manual **Page Table Walk** (PGD -> P4D -> PUD -> PMD -> PTE) to resolve a process's virtual address to a raw physical address.
3. **GKI Compliance**: Built against the **Android Common Kernel (ACK)**, ensuring compatibility with the strict Kernel Module Interface (KMI) introduced in Android 12 (Kernel 5.10+).

---

#### 🔍 Implementation Details
* **Communication**: Uses `Generic Netlink` (Family: `MY_GENL_BUS`) for efficient asynchronous data transfer.
* **Atomicity**: Uses `kmap_atomic` for high-speed, interrupt-safe mapping.
* **Access Control**: Supports `READ_PHYS` and `WRITE_PHYS` operations.

### 🛠️ Technical Specifications
* **Target Architecture**: ARM64 (aarch64)
* **Kernel Base**: Linux 5.10.x / 6.1.x (Android GKI)
* **Toolchain**: LLVM/Clang (Official Google Toolchain)

---

## 🏗️ Detailed Build Instructions

To build this module, you use the Linux Kernel Build System (Kbuild). The command is complex because it must point the compiler to the Android-specific toolchain and the prepared kernel source.

### The Build Command
Navigate to your module directory (`~/physmem`) and run:

```bash
make -C ~/android_kernel/common M=$PWD \
    ARCH=arm64 \
    LLVM=1 LLVM_IAS=1 \
    CLANG_TRIPLE=aarch64-linux-gnu- \
    CROSS_COMPILE=aarch64-linux-android- \
    modules
