#include <net/genetlink.h>

#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/highmem.h>
#include <linux/sched/mm.h>
#include <linux/slab.h>

#define MY_FAMILY_NAME "MY_GENL_BUS"


enum {
    MY_ATTR_UNSPEC,
    MY_ATTR_DATA,
    __MY_ATTR_MAX,
};
#define MY_ATTR_MAX (__MY_ATTR_MAX - 1)


enum {
    MY_CMD_UNSPEC,
    MY_CMD_READ_PHYS,
    MY_CMD_WRITE_PHYS,
    __MY_CMD_MAX,
};


struct phys_req {
    uint32_t pid;
    uint64_t vaddr;
    uint64_t size;
    unsigned char buffer[256];
} __attribute__((packed));


static int handle_phys_op(struct sk_buff* skb, struct genl_info* info);


static const struct genl_ops my_genl_ops[] = {
    {
        .cmd = MY_CMD_READ_PHYS,
        .doit = handle_phys_op,
    },
    {
        .cmd = MY_CMD_WRITE_PHYS,
        .doit = handle_phys_op,
    },
};

static struct genl_family my_genl_family = {
    .name = MY_FAMILY_NAME,
    .version = 1,
    .maxattr = MY_ATTR_MAX,
    .module = THIS_MODULE,
    .ops = my_genl_ops,
    .n_ops = ARRAY_SIZE(my_genl_ops),
};


static unsigned long v_to_p(struct task_struct* task, unsigned long vaddr) {
    pgd_t* pgd;
    p4d_t* p4d;
    pud_t* pud;
    pmd_t* pmd;
    pte_t* pte;
    struct mm_struct* mm = task->mm;

    if (!mm) return 0;

    pgd = pgd_offset(mm, vaddr);
    if (pgd_none(*pgd) || pgd_bad(*pgd)) return 0;

    p4d = p4d_offset(pgd, vaddr);
    if (p4d_none(*p4d) || p4d_bad(*p4d)) return 0;

    pud = pud_offset(p4d, vaddr);
    if (pud_none(*pud) || pud_bad(*pud)) return 0;

    pmd = pmd_offset(pud, vaddr);
    if (pmd_none(*pmd) || pmd_bad(*pmd)) return 0;

    pte = pte_offset_kernel(pmd, vaddr);
    if (pte_none(*pte)) return 0;

    return (pte_pfn(*pte) << PAGE_SHIFT) | (vaddr & ~PAGE_MASK);
}


static int handle_phys_op(struct sk_buff* skb, struct genl_info* info) {
    struct phys_req *req, *res_data;
    struct nlattr *na;
    struct task_struct* task;
    struct sk_buff* reply;
    void *msg_head, *mapped_page;
    struct page* page;
    unsigned long paddr;
    size_t sz;
    int is_write = (info->genlhdr->cmd == MY_CMD_WRITE_PHYS);

    if (!info->attrs[MY_ATTR_DATA]) return -EINVAL;
    req = nla_data(info->attrs[MY_ATTR_DATA]);

    
    task = get_pid_task(find_get_pid(req->pid), PIDTYPE_PID);
    if (!task) return -ESRCH;

   
    paddr = v_to_p(task, req->vaddr);
    if (!paddr || !pfn_valid(paddr >> PAGE_SHIFT)) {
        put_task_struct(task);
        return -EFAULT;
    }

  
    page = pfn_to_page(paddr >> PAGE_SHIFT);
    mapped_page = kmap_atomic(page);
    
    
    sz = req->size > 256 ? 256 : req->size;


    if (is_write) {
        memcpy((char*)mapped_page + (paddr & ~PAGE_MASK), req->buffer, sz);
    } else {
        memcpy(req->buffer, (char*)mapped_page + (paddr & ~PAGE_MASK), sz);
    }

    kunmap_atomic(mapped_page);
    put_task_struct(task);

  
    if (!is_write) {
        reply = genlmsg_new(NLMSG_GOODSIZE, GFP_KERNEL);
        if (!reply) return -ENOMEM;

        msg_head = genlmsg_put(reply, info->snd_portid, info->snd_seq, &my_genl_family, 0, MY_CMD_READ_PHYS);
        if (!msg_head) {
            nlmsg_free(reply);
            return -EMSGSIZE;
        }

        na = nla_reserve(reply, MY_ATTR_DATA, sizeof(struct phys_req));
        if (!na) {
            nlmsg_free(reply);
            return -EMSGSIZE;
        }
        
        res_data = nla_data(na);
        memcpy(res_data, req, sizeof(struct phys_req));

        genlmsg_end(reply, msg_head);
        return genlmsg_reply(reply, info);
    }

    return 0;
}


static int __init my_driver_init(void) {
    int rc = genl_register_family(&my_genl_family);
    if (rc != 0) {
        pr_err("Genl Driver: Failed to register family\n");
        return rc;
    }
    pr_info("Ghost Driver Loaded: %s\n", MY_FAMILY_NAME);
    return 0;
}


static void __exit my_driver_exit(void) {
    genl_unregister_family(&my_genl_family);
    pr_info("Ghost Driver Unloaded\n");
}

module_init(my_driver_init);
module_exit(my_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Paranoia");