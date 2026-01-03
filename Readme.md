🛰️ Android Physical Memory Read/Write Kernel Module (GKI)
This project is a highí-performance Linux kernel module (.ko) designed for Android devices running GKI (Generic Kernel Image).
It provides a specialized interface to perform direct physical memory operations, bypassing standard virtual memory restrictions for debugging and memory analysis purposes.

⚙️ How It Works
The module creates a character device (or uses an IOCTL interface) to bridge the gap between user-space and the system's physical RAM.

1. 
Memory Mapping: Since the CPU uses a Virtual Memory Unit (MMU), the module uses kernel functions like kmap_atomic or vmap to map specific physical addresses into the kernel's virtual address space temporarily.
2.
Kernel-to-User Data Transfer: It utilizes copy_to_user and copy_from_user to safely move data between the restricted kernel memory space and your user-space application.
3.
GKI Compliance: The module is built against the Android Common Kernel (ACK), ensuring compatibility with the strict Kernel Module Interface (KMI) introduced in Android 12 (Kernel 5.10+).

🛠️ Technical Specifications
Target Architecture: ARM64 (aarch64)

Kernel Base: Linux 5.10.x / 6.1.x (Android GKI)

Toolchain: LLVM/Clang (as per Google's official build requirements)

Key Features:

Read/Write physical memory bypassing PAGEMAP.

Support for Large Pages and specialized memory offsets.

Low-latency execution for real-time memory monitoring.

⚠️ Disclaimer
This project is for educational and research purposes only. Directly manipulating physical memory can lead to system instability, data corruption, or permanent hardware "soft-brick" if handled incorrectly. Use only on development devices with an unlocked bootloader.
