#include <compiler.h>
#include <kpmodule.h>
#include <linux/printk.h>
#include <uapi/asm-generic/unistd.h>
#include <linux/uaccess.h>
#include <syscall.h>
#include <kputils.h>

KPM_NAME("kpm-debugger-bypass");
KPM_VERSION("1.0.1");
KPM_LICENSE("GPL v2");
KPM_AUTHOR("Kahllo");
KPM_DESCRIPTION("Block /proc/self/status and /proc/self/task/*/status via openat");

enum hook_type hook_type = NONE;

// Simple local strcmp (no libc symbols)
static int local_strcmp(const char *s1, const char *s2)
{
    while (*s1 && (*s1 == *s2)) {
        ++s1; ++s2;
    }
    return *(unsigned char *)s1 - *(unsigned char *)s2;
}

// Simple local strstr (no kernel exports)
static int contains_status_path(const char *path)
{
    if (!path) return 0;

    // Match exact /proc/self/status
    if (local_strcmp(path, "/proc/self/status") == 0)
        return 1;

    // Match /proc/self/task/<tid>/status
    if (local_strcmp(path, "/proc/self/task/") < 0)
        return 0;

    const char *sub = path;
    while (*sub) {
        if (*sub == '/' &&
            sub[1] == 's' && sub[2] == 't' && sub[3] == 'a' &&
            sub[4] == 't' && sub[5] == 'u' && sub[6] == 's' &&
            (sub[7] == '\0' || sub[7] == '/')) {
            return 1;
        }
        sub++;
    }

    return 0;
}

void before_openat(hook_fargs4_t *args, void *udata)
{
    const char __user *filename = (typeof(filename))syscall_argn(args, 1);
    char buf[256] = {0};

    if (compat_strncpy_from_user(buf, filename, sizeof(buf)) <= 0)
        return;

    if (contains_status_path(buf)) {
        pr_info("[kpm-debugger-bypass] Blocked access to: %s\n", buf);
        args->ret = -ENOENT;
        args->skip_origin = 1; // Only if your KPM version supports this
    }
}

static long debugger_bypass_init(const char *args, const char *event, void *__user reserved)
{
    pr_info("[kpm-debugger-bypass] init\n");

    hook_err_t err = inline_hook_syscalln(__NR_openat, 4, before_openat, 0, 0);
    if (err) {
        pr_err("[kpm-debugger-bypass] Failed to hook openat: %d\n", err);
    } else {
        pr_info("[kpm-debugger-bypass] Hook installed successfully\n");
        hook_type = INLINE_CHAIN;
    }

    return 0;
}

static long debugger_bypass_exit(void *__user reserved)
{
    if (hook_type == INLINE_CHAIN) {
        inline_unhook_syscalln(__NR_openat, before_openat, 0);
        pr_info("[kpm-debugger-bypass] openat unhooked\n");
    }

    return 0;
}

KPM_INIT(debugger_bypass_init);
KPM_EXIT(debugger_bypass_exit);
