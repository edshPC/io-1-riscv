#include <linux/module.h>
#include <linux/version.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

static dev_t first;
static struct cdev c_dev; 
static struct class *cl;

static int all_spaces[1024];
static int last_write = 0;

static int my_open(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: open()\n");
  return 0;
}

static int my_close(struct inode *i, struct file *f)
{
  printk(KERN_INFO "Driver: close()\n");
  return 0;
}

static ssize_t my_read(struct file *f, char __user *buf, size_t len, loff_t *off)
{
  printk(KERN_INFO "Driver: read()\n");
  char kbuf[1024];
  size_t i = 0, pos = 0;
  if (*off >= last_write) {
    return 0;
  }
  while (i < last_write) {
    pos += sprintf(kbuf + pos, "Input spaces: %d\n", all_spaces[i]);
    i++;
  }
  kbuf[pos] = '\0';
  printk(kbuf);
  if (copy_to_user(buf, kbuf, pos)) {
    return -EFAULT;
  }
  *off += pos;
  return pos;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t len, loff_t *off)
{
  printk(KERN_INFO "Driver: write()\n");
  char kbuf[1024];
  copy_from_user(kbuf, buf, len);
  int spaces = 0;
  size_t i = 0;
  while (i < len) {
    if (kbuf[i] == ' ') {spaces++;}
    i++;
  }
  all_spaces[last_write++] = spaces;
  printk(KERN_INFO "Spaces: %d\n", spaces);
  return len;
}

static struct file_operations mychdev_fops =
{
  .owner = THIS_MODULE,
  .open = my_open,
  .release = my_close,
  .read = my_read,
  .write = my_write
};
 
static int __init ch_drv_init(void)
{
    printk(KERN_INFO "Hello!\n");
    if (alloc_chrdev_region(&first, 0, 1, "ch_dev") < 0)
    {
    return -1;
    }
    if ((cl = class_create(THIS_MODULE, "chardrv")) == NULL)
    {
    unregister_chrdev_region(first, 1);
    return -1;
    }
    if (device_create(cl, NULL, first, NULL, "var2") == NULL)
    {
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
    }
    cdev_init(&c_dev, &mychdev_fops);
    if (cdev_add(&c_dev, first, 1) == -1)
    {
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    return -1;
    }
    return 0;
}
 
static void __exit ch_drv_exit(void)
{
    cdev_del(&c_dev);
    device_destroy(cl, first);
    class_destroy(cl);
    unregister_chrdev_region(first, 1);
    printk(KERN_INFO "Bye!!!\n");
}
 
module_init(ch_drv_init);
module_exit(ch_drv_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Author");
MODULE_DESCRIPTION("The first kernel module");

