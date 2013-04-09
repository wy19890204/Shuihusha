1、水浒杀默认使用rcc资源文件。如果不想使用，修改Shuihusha.pro，将第8行行首加上#注释掉，重新编译即可。
2、修改rcc.bat，将第5行的路径修改为你本地QT的rcc.exe路径（可在QT目录下搜索rcc.exe得到）。
3、将所有的qrc文件拖动到rcc.bat图标上，即可生成程序需要的同名rcc文件
4、skin.qrc不需做此处理。