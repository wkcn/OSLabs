实验6（版本1, 编译版本号：462）：
系统镜像在build/disk.img
实验报告在report文件夹

这次实验更正了之前fork函数的返回值错误问题
新增了线程thread_create和thread_join函数, 能够实现多线程矩阵乘法
实现了进程优先级



可用指令：
指令名称	功能
r	回到用户程序模式
top 列出所有进程及详细状态
int 调用某个中断（支持33h,34h,35h,36h）, 如int 33h
kill 杀死某个进程
wake 唤醒某个进程
suspend 挂起某个进程

[*新增指令*]
pr 设置进程优先级, 用法:pr 1 5, 即设置PID为1的进程优先级为5

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
**测试多线程**:
mat   一个多线程矩阵乘法测试
**测试IO端口, 可用于进程通信：
porttest 使用0号端口读写屏幕IO
**测试fork():
fork1, fork2, alpha
测试大小写转换（使用了系统中断）：
alpha

更多程序可以通过ls列出

输入数字12345也可运行程序
当连续输入数字时使用批处理
单独输入，为分时模式

不同虚拟机平台运行情况有差异
分时系统已经非常稳定！

谢谢:-)
