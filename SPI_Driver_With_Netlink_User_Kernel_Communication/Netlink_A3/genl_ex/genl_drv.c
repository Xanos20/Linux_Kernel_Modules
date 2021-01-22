#include <linux/version.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/skbuff.h>
#include <linux/netlink.h>
#include <linux/timer.h>
#include <linux/export.h>
#include <net/genetlink.h>

#include "genl_ex.h"

static struct timer_list timer;
static struct genl_family genl_test_family;



typedef struct {
    uint8_t led[8];
} PATTERN;

struct user_to_kernel {
    int is_ioctl;
    int is_distance_request;
    int is_display_pattern;
    int is_latest_dist;
    int trigger;
    int echo;
    int chip_select;
    PATTERN pattern;
    unsigned long long latest_dist;
    int pad1;
    int pad2;
    int pad3;
    int pad4;
    char null_byte;
};

struct user_to_kernel* data;



/*
  Assembly code to get the Time Stamp Counter
 */
static __always_inline unsigned long long get_native_read_tsc_here(void) {
    DECLARE_ARGS(val, low, high);
    asm volatile("rdtsc" : EAX_EDX_RET(val, low, high));
    return (unsigned long long) EAX_EDX_VAL(val, low, high);
}

/*
Sensor Globals
*/






































static void greet_group(unsigned int group)
{   
    void *hdr;
    int res, flags = GFP_ATOMIC;
    char msg[GENL_TEST_ATTR_MSG_MAX];
    struct sk_buff* skb = genlmsg_new(NLMSG_DEFAULT_SIZE, flags);

    if (!skb) {
        printk(KERN_ERR "%d: OOM!!", __LINE__);
        return;
    }

    hdr = genlmsg_put(skb, 0, 0, &genl_test_family, flags, GENL_TEST_C_MSG);
    if (!hdr) {
        printk(KERN_ERR "%d: Unknown err !", __LINE__);
        goto nlmsg_fail;
    }

    snprintf(msg, GENL_TEST_ATTR_MSG_MAX, "Hello group %s\n",
            genl_test_mcgrp_names[group]);

    res = nla_put_string(skb, GENL_TEST_ATTR_MSG, msg);
    if (res) {
        printk(KERN_ERR "%d: err %d ", __LINE__, res);
        goto nlmsg_fail;
    }

    genlmsg_end(skb, hdr);
    genlmsg_multicast(&genl_test_family, skb, 0, group, flags);
    return;

nlmsg_fail:
    genlmsg_cancel(skb, hdr);
    nlmsg_free(skb);
    return;
}




static void send_distance_to_user(struct sk_buff* rec_skb, struct genl_info* info)
{   
    void *hdr;
    int err;
    int res, flags = GFP_ATOMIC;
    char msg[GENL_TEST_ATTR_MSG_MAX];
    printk("TRY: send_distance_to_user\n");
    struct sk_buff* skb = genlmsg_new(NLMSG_DEFAULT_SIZE, flags);

    unsigned long long distance = 255;

    if (!skb) {
        printk(KERN_ERR "%d: ERROR: genlmsg_new", __LINE__);
        return;
    }

    hdr = genlmsg_put(skb, 0, 0, &genl_test_family, flags, GENL_TEST_C_MSG);
    if (!hdr) {
        printk(KERN_ERR "ERROR=%d: genlmsg_put !", __LINE__);
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return;
    }

    err = nla_put(skb, GENL_TEST_ATTR_MSG, sizeof(unsigned long long), &distance);
    if(err < 0) {
        printk("ERROR=%d: nla_put\n", err);
        genlmsg_cancel(skb, hdr);
        nlmsg_free(skb);
        return;
    }

    genlmsg_end(skb, hdr);
    //genlmsg_multicast(&genl_test_family, skb, 0, group, flags);
    printk("KVAR: user portid = %d\n", info->snd_portid);
    printk("TRY: genlmsg_unicast\n");
    err = nlmsg_unicast(rec_skb->sk, skb, info->snd_portid);
    if(err < 0) {
        printk("ERROR=%d: genlmsg_unicast\n", err);
    }
    printk("INFO: Sent\n");
    return;

}












#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,14,0)
void genl_test_periodic(unsigned long data)
#else
void genl_test_periodic(struct timer_list *unused)
#endif
{
    greet_group(GENL_TEST_MCGRP0);
    greet_group(GENL_TEST_MCGRP1);
    greet_group(GENL_TEST_MCGRP2);

    mod_timer(&timer, jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL));
}







static int process_config(struct sk_buff* skb, struct genl_info* info) {

    return 0;
}


static int genl_test_rx_msg(struct sk_buff* skb, struct genl_info* info)
{
    if (!info->attrs[GENL_TEST_ATTR_MSG]) {
        printk(KERN_ERR "ERROR: empty message from %d!!\n",
            info->snd_portid);
        printk(KERN_ERR "%p\n", info->attrs[GENL_TEST_ATTR_MSG]);
        return -EINVAL;
    }

    /*
    printk(KERN_NOTICE "%u says %s \n", info->snd_portid,
        (char*)nla_data(info->attrs[GENL_TEST_ATTR_MSG]));
        */
    data = (struct user_to_kernel*) nla_data(info->attrs[GENL_TEST_ATTR_MSG]);
    printk("KVAR: Trigger = %d\n", data->trigger);
    printk("KVAR: Data Length info = %d\n", info->attrs[GENL_TEST_ATTR_MSG]->nla_len);
    printk("KINFO: Sizeof struct = %d\n", sizeof(struct user_to_kernel));
    //printk()
    send_distance_to_user(skb, info);
    return 0;
}



static const struct genl_ops genl_test_ops[] = {
    {
        .cmd = GENL_TEST_C_MSG,
        .policy = genl_test_policy,
        .doit = genl_test_rx_msg,
        .dumpit = NULL,
    },
};



static const struct genl_multicast_group genl_test_mcgrps[] = {
    [GENL_TEST_MCGRP0] = { .name = GENL_TEST_MCGRP0_NAME, },
    [GENL_TEST_MCGRP1] = { .name = GENL_TEST_MCGRP1_NAME, },
    [GENL_TEST_MCGRP2] = { .name = GENL_TEST_MCGRP2_NAME, },
};



static struct genl_family genl_test_family = {
    .name = GENL_TEST_FAMILY_NAME,
    .version = 1,
    .maxattr = GENL_TEST_ATTR_MAX,
    .netnsok = false,
    .module = THIS_MODULE,
    .ops = genl_test_ops,
    .n_ops = ARRAY_SIZE(genl_test_ops),
    .mcgrps = genl_test_mcgrps,
    .n_mcgrps = ARRAY_SIZE(genl_test_mcgrps),
};



static int __init genl_test_init(void)
{
    int rc;

    printk(KERN_INFO "genl_test: initializing netlink\n");

    rc = genl_register_family(&genl_test_family);
    if (rc)
        goto failure;

#if LINUX_VERSION_CODE <= KERNEL_VERSION(4,14,0)
    init_timer(&timer);
    timer.data = 0;
    timer.function = genl_test_periodic;
    timer.expires = jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL);
    add_timer(&timer);
#else
    timer_setup(&timer, genl_test_periodic, 0);
    mod_timer(&timer, jiffies + msecs_to_jiffies(GENL_TEST_HELLO_INTERVAL));
#endif

    return 0;

failure:
    printk(KERN_DEBUG "genl_test: error occurred in %s\n", __func__);
    return -EINVAL;
}
module_init(genl_test_init);



static void genl_test_exit(void)
{
    del_timer(&timer);
    genl_unregister_family(&genl_test_family);
}
module_exit(genl_test_exit);

MODULE_AUTHOR("Ahmed Zaki <anzaki@gmail.com>");
MODULE_LICENSE("GPL");
