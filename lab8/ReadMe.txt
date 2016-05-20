实验8（版本0.7, 编译版本号：1214）：
多用户与磁盘读写

系统镜像为MiraiOS.img
实验报告在report文件夹

本次实验亮点：
1. 实现了多用户(终端)机制
2. 实现了软盘的读写, open, write, read, seekp, seekg


可用指令：
指令名称	功能
r	回到用户程序模式
top 列出当前用户的进程及详细状态
topa 列出所有用户的进程及详细状态 [*新增指令*]
uid 显示当前用户ID
kill 杀死某个进程
wake 唤醒某个进程
suspend 挂起某个进程

文件读写指令
[*新增指令*]
open 打开一个文件
write 写入数据到文件
read 从文件中读取数据
seekp 改变写入指针， 如seekp 10
seekg 改变读取指针, 如seekg 0
file 查看文件状态
文件操作指令
[*新增指令*]
rm 可恢复式删除
mv 重命名文件 ex: mv source target
cp 拷贝文件 ex: cp source target

mem 查看内存管理链表信息
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

[**新增按键**]
Alt+Num: 切换用户，如Alt + 1, Alt + 2, Alt + 3, Alt + 4

用户程序：
ls,bpb,bs,
wkcn1,wkcn2,wkcn3,wkcn4,kan,box

[** 保存屏幕到文件screen.txt **]
screen

[信号量测试]
fruit
[内存管理测试]
testnew   (注意后面要带参数，表示该用户进程可动态申请内存的大小)
测试系统调用:
testport
测试多线程:
mat   一个多线程矩阵乘法测试
letter 数一个字符串有多少个字母
测试IO端口, 可用于进程通信：
porttest 使用0号端口读写屏幕IO
测试fork():
fork1, fork2, alpha
测试大小写转换（使用了系统中断）
alpha

更多程序可以通过ls列出

输入数字12345也可运行程序
当连续输入数字时使用批处理
单独输入，为分时模式

不同虚拟机平台运行情况有差异

谢谢:-)
