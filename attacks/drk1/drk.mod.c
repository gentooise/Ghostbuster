#include <linux/module.h>
#include <linux/vermagic.h>
#include <linux/compiler.h>

MODULE_INFO(vermagic, VERMAGIC_STRING);

__visible struct module __this_module
__attribute__((section(".gnu.linkonce.this_module"))) = {
	.name = KBUILD_MODNAME,
	.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	.exit = cleanup_module,
#endif
	.arch = MODULE_ARCH_INIT,
};

static const struct modversion_info ____versions[]
__used
__attribute__((section("__versions"))) = {
	{ 0x380122c1, __VMLINUX_SYMBOL_STR(module_layout) },
	{ 0x3df37a55, __VMLINUX_SYMBOL_STR(param_ops_charp) },
	{ 0xe7dbffbf, __VMLINUX_SYMBOL_STR(param_ops_int) },
	{ 0xf4a5937b, __VMLINUX_SYMBOL_STR(unregister_hw_breakpoint) },
	{ 0x84b6a986, __VMLINUX_SYMBOL_STR(register_user_hw_breakpoint) },
	{ 0x48883108, __VMLINUX_SYMBOL_STR(pid_task) },
	{ 0x6c1cc8db, __VMLINUX_SYMBOL_STR(find_vpid) },
	{ 0xfa2a45e, __VMLINUX_SYMBOL_STR(__memzero) },
	{ 0xb742fd7, __VMLINUX_SYMBOL_STR(simple_strtol) },
	{ 0x2e5810c6, __VMLINUX_SYMBOL_STR(__aeabi_unwind_cpp_pr1) },
	{ 0x27e1a049, __VMLINUX_SYMBOL_STR(printk) },
	{ 0xb1ad28e0, __VMLINUX_SYMBOL_STR(__gnu_mcount_nc) },
};

static const char __module_depends[]
__used
__attribute__((section(".modinfo"))) =
"depends=";


MODULE_INFO(srcversion, "4618ABC87D887F6F5DFBFD7");
