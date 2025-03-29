# 🔐 kpm-debugger-bypass

Bypass Android anti-debug protections using a lightweight **KPM (KernelPatch Module)**.

This module hooks the `openat` syscall in the kernel and blocks access to:

- `/proc/self/status`
- `/proc/self/task/*/status`

These files are commonly used by apps to detect debuggers. With this module loaded, tools like **LLDB** can attach without triggering detection.

---

## 🚀 Usage

1. Load the module using **APatch KPM Manager**  
   📘 [Usage Guide →](https://apatch.dev/kpm-usage-guide.html#embed)

2. After loading the module, check logs with:

   ```sh
   dmesg -w | grep "kpm-debugger-bypass"

---

# 🧠 Learn More

📝 Full write-up with deep explanation, code snippets, and testing examples:

👉 Read the blog on [Medium](https://medium.com/@omerqw23451/c4f2f92bd6fa)

