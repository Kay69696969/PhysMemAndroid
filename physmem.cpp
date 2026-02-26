#include "physmem.h"
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/genetlink.h>
#include <unistd.h>
#include <cstring>
#include <fstream>
#include <dirent.h>
#include <vector>
#include <string>

// --- Netlink User-space Macros ---
#define NLA_ALIGNTO      4
#define NLA_ALIGN(len)   (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN       ((int) NLA_ALIGN(sizeof(struct nlattr)))

inline int nla_attr_size(int payload) { return NLA_HDRLEN + payload; }
inline int nla_total_size(int payload) { return NLA_ALIGN(nla_attr_size(payload)); }
inline void* nla_data(struct nlattr* nla) { return (void*)((char*)(nla)+NLA_HDRLEN); }

// --- Global Driver State ---
int nl_fd = -1;
int family_id = -1;

struct phys_req {
    uint32_t pid;
    uint64_t vaddr;
    uint64_t size;
    unsigned char buffer[256];
} __attribute__((packed));

// --- Internal Helper: Get Driver Family ID ---
int get_family_id(int fd, const char* name) {
    struct { struct nlmsghdr nlh; struct genlmsghdr gnlh; char buf[256]; } req;
    memset(&req, 0, sizeof(req));
    req.nlh.nlmsg_len = NLMSG_LENGTH(GENL_HDRLEN + nla_total_size(strlen(name) + 1));
    req.nlh.nlmsg_type = GENL_ID_CTRL;
    req.nlh.nlmsg_flags = NLM_F_REQUEST;
    req.gnlh.cmd = CTRL_CMD_GETFAMILY;
    req.gnlh.version = 1;
    struct nlattr* na = (struct nlattr*)((char*)NLMSG_DATA(&req.nlh) + GENL_HDRLEN);
    na->nla_type = CTRL_ATTR_FAMILY_NAME;
    na->nla_len = nla_attr_size(strlen(name) + 1);
    strcpy((char*)nla_data(na), name);
    if (send(fd, &req, req.nlh.nlmsg_len, 0) < 0) return -1;
    char res[1024];
    int len = recv(fd, res, sizeof(res), 0);
    if (len < 0) return -1;
    struct nlmsghdr* nlh = (struct nlmsghdr*)res;
    struct nlattr* attr = (struct nlattr*)((char*)NLMSG_DATA(nlh) + GENL_HDRLEN);
    while ((char*)attr < res + len) {
        if (attr->nla_type == CTRL_ATTR_FAMILY_ID) return *(uint16_t*)nla_data(attr);
        attr = (struct nlattr*)((char*)attr + NLA_ALIGN(attr->nla_len));
    }
    return -1;
}

namespace physmem {

    // --- Process Helpers  ---

    int get_pid(const char* package_name) {
        DIR* dir = opendir("/proc");
        if (!dir) return 0;
        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            int pid = atoi(entry->d_name);
            if (pid > 0) {
                char path[128];
                snprintf(path, sizeof(path), "/proc/%d/cmdline", pid);
                std::ifstream cmdline(path);
                std::string line;
                if (std::getline(cmdline, line)) {
                    if (line.find(package_name) != std::string::npos) {
                        closedir(dir);
                        return pid;
                    }
                }
            }
        }
        closedir(dir);
        return 0;
    }

    uintptr_t get_module_base(int pid, const char* module_name) {
        char path[128];
        snprintf(path, sizeof(path), "/proc/%d/maps", pid);
        std::ifstream maps(path);
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find(module_name) != std::string::npos) {
                return std::stoull(line.substr(0, line.find("-")), nullptr, 16);
            }
        }
        return 0;
    }

    // --- Driver Communication ---

    bool InitDriver() {
        if (nl_fd != -1 && family_id > 0) return true;
        if (nl_fd == -1) nl_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_GENERIC);
        if (nl_fd < 0) return false;
        struct timeval tv = { 0, 50000 };
        setsockopt(nl_fd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
        family_id = get_family_id(nl_fd, "MY_GENL_BUS");
        return family_id > 0;
    }

    bool ReadRaw(int pid, uintptr_t addr, void* buf, size_t sz) {
        if (!InitDriver() || sz > 256) return false;
        struct {
            struct nlmsghdr nlh;
            struct genlmsghdr gnlh;
            struct { struct nlattr attr; phys_req data; } __attribute__((packed)) p;
        } req;
        memset(&req, 0, sizeof(req));
        req.nlh.nlmsg_len = sizeof(req);
        req.nlh.nlmsg_type = family_id;
        req.nlh.nlmsg_flags = NLM_F_REQUEST;
        req.gnlh.cmd = 1; // READ
        req.p.attr.nla_type = 1;
        req.p.attr.nla_len = nla_attr_size(sizeof(phys_req));
        req.p.data.pid = pid; req.p.data.vaddr = addr; req.p.data.size = sz;

        if (send(nl_fd, &req, sizeof(req), 0) < 0) return false;

        char res[1024];
        int len = recv(nl_fd, res, sizeof(res), 0);
        if (len <= 0) return false;

        struct nlattr* res_attr = (struct nlattr*)((char*)NLMSG_DATA(res) + GENL_HDRLEN);
        phys_req* res_data = (phys_req*)nla_data(res_attr);
        memcpy(buf, res_data->buffer, sz);
        return true;
    }

    bool WriteRaw(int pid, uintptr_t addr, void* buf, size_t sz) {
        if (!InitDriver() || sz > 256) return false;
        struct {
            struct nlmsghdr nlh;
            struct genlmsghdr gnlh;
            struct { struct nlattr attr; phys_req data; } __attribute__((packed)) p;
        } req;
        memset(&req, 0, sizeof(req));
        req.nlh.nlmsg_len = sizeof(req);
        req.nlh.nlmsg_type = family_id;
        req.nlh.nlmsg_flags = NLM_F_REQUEST;
        req.gnlh.cmd = 2; // WRITE
        req.p.attr.nla_type = 1;
        req.p.attr.nla_len = nla_attr_size(sizeof(phys_req));
        req.p.data.pid = pid; req.p.data.vaddr = addr; req.p.data.size = sz;
        memcpy(req.p.data.buffer, buf, sz);
        return send(nl_fd, &req, sizeof(req), 0) > 0;
    }

}
