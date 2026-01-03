# 🛰️ Android Physical Memory Read/Write Kernel Module (GKI)

This project is a high-performance Linux kernel module (`.ko`) designed for Android devices running **GKI (Generic Kernel Image)**. It provides a specialized interface to perform direct physical memory operations, bypassing standard virtual memory restrictions for debugging, research, and memory analysis purposes.

---

### ⚙️ How It Works

The module creates a communication bridge using the Generic Netlink protocol to handle requests between user-space and the system's physical RAM.

1. **Memory Mapping**: Since the CPU uses a Virtual Memory Unit (MMU), the module uses `kmap_atomic` to map specific physical addresses into the kernel's virtual address space temporarily.
2. **Address Translation**: It implements a manual **Page Table Walk** (PGD -> P4D -> PUD -> PMD -> PTE) to resolve a process's virtual address to a raw physical address.

3. **GKI Compliance**: Built against the **Android Common Kernel (ACK)**, ensuring compatibility with the strict Kernel Module Interface (KMI) introduced in Android 12 (Kernel 5.10+).

---

#### 🔍 Implementation Details

* **Communication**: Uses `Generic Netlink` (Family: `MY_GENL_BUS`) for efficient asynchronous data transfer.
* **Atomicity**: Uses `kmap_atomic` for high-speed, interrupt-safe mapping of physical pages.
* **Access Control**: Supports both `READ_PHYS` and `WRITE_PHYS` operations via IOCTL-like commands over Netlink.

### 🛠️ Technical Specifications

* **Target Architecture**: ARM64 (aarch64)
* **Kernel Base**: Linux 5.10.x / 6.1.x (Android GKI)
* **Toolchain**: LLVM/Clang (Official Google Toolchain as per GKI requirements)

#### ✨ Key Features

* **Direct Access**: Read/Write physical memory bypassing `PAGEMAP`.
* **Efficiency**: Support for Large Pages and specialized memory offsets.
* **Performance**: Low-latency execution for real-time memory monitoring.

---

## 🏗️ Detailed Build Instructions

To build this kernel module, you must precisely follow these steps to ensure the `vermagic` and `KMI` match your target device.

### 1. Environment & Path Setup

You need a Linux environment (Ubuntu 22.04 LTS or WSL2). First, install the necessary build tools:

```bash
sudo apt update && sudo apt install -y build-essential bc flex bison libssl-dev libelf-dev
