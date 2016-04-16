实验5（版本1）：
系统镜像disk.img放在build文件夹中
实验报告在report文件夹中
进入系统后输入help能获得对应帮助。

我计划在版本2中使用系统调用进行进程间通信

系统分为Shell模式（可以输入命令）和用户程序模式
默认用户程序会在切换到Shell模式下全部挂起
用户程序也可以在Shell模式下运行， 需要在Shell模式下用wake 进程id的方式唤醒。
Ouch!Ouch!只能在用户程序模式按键显示

可用指令：
指令名称	功能
r	回到用户程序模式
top 列出所有进程及详细状态
int 调用某个中断（支持33h,34h,35h,36h）, 如int 33h
kill 杀死某个进程
wake 唤醒某个进程
suspend 挂起某个进程
ls	列出所有用户程序信息(大小时间占用簇)
bpb	显示BPB
bs	显示EBPB
cls	清屏
killall	杀死所有用户进程
uname	显示操作系统信息

特殊按键：
ESC: 返回Shell但是不杀死进程
Ctrl+C: 在非Shell模式进行清屏
Ctrl+Z: 返回Shell并且杀死所有进程

用户程序：
ls,bpb,bs,hello,
wkcn1,wkcn2,wkcn3,wkcn4,kan,box
clock,ouch,inttest

输入数字12345也可运行程序
当连续输入数字时使用批处理
单独输入，为分时模式

不同虚拟机平台运行情况有差异
分时系统已经非常稳定！

谢谢:-)
