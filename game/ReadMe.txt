欢乐QQ堂 ASM版 ver 1.0

制作：吴侃 14348134 (wkcn)
除了封面为自己绘图，其余游戏素材来自游戏“QQ堂”

***
游戏文件为: HappyQQT.img
报告在：report文件夹中
***

操作方式：

方向键移动
空格开始游戏，或者在结束游戏时返回开始界面

剧情：
年兽来糖果王国踢足球了，你要打败它！
界面上方为年兽的血量，共5 HP
你可以踢足球:-)
在驱逐年兽的同时，要避免足球炸弹的伤害！

编译方式：
在Linux系统下输入sudo make

强烈推荐使用VMware Workstation虚拟机以保持游戏的流畅性
make run:使用qemu运行
make bochs:使用bochs运行 （极不推荐，只能用来调试）

文件夹：
include 头文件、包含文件
source  源文件
pic     图片
res     转换格式后的图片资源
tools   一些工具
