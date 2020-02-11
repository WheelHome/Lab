
#*************************** CMake语法的主体框架 ***************************#

command(arg1 arg2...)                                                                                         # 运行命令
set(var_name var_value)                                                                                     # 定义变量，或者给已经存在的变量赋值
command(arg1 ${var_name})                                                                           # 使用变量

#*************************** 工程配置部分**********************#
cmake_minimum_required(VERSION num)                                                 # CMake最低版本号要求
project(cur_project_name)                                                                                  # 项目信息
set(CMAKE_CXX_FLAGS "xxx" )                                                                           # 设定编译器模式，如-std=c++11
set(CMAKE_BUILD_TYPE "xxx" )                                                                         # 设定编译模式，如Debug/Release

#************************** 依赖执行部分 **********************#
find_package(std_lib_name VERSION REQUIRED)                                     # 引入外部依赖
add_library(<name> [lib_type] source1)                                                         # 生成库类型(动态，静态)
include_directories(${std_lib_name_INCLUDE_DIRS})                            # 指定include路径，放在add_executable前面
add_executable(cur_projecy_name XXX.cpp)                                              # 指定生成目标
target_link_libraries(${std_lib_name_LIBRARIES})                                    # 指定libraries路径，放在add_executable后面

#************************** 其他部分 *************************#
function(function_name arg)                                                                               # 定义一个函数
add_subdirectory(dir)                                                                                             # 添加一个子目录
AUX_SOURCE_DIRECTORY(. SRC_LIST)                                                           # 查找当前目录所有文件，并保存到SRC_LIST变量中
FOREACH(one_dir ${SRC_LIST})
    MESSAGE(${one_dir})                                                                                          #使用message进行打印
ENDFOREACH(one_dir)
endfunction(function_name arg)

#************************ 判断控制部分 **********************#
if(expression)                                                                                                               #不区分大小写，并视同"#"来进行注释
    COMMAND(AGRS)
ELSE(expression)
    COMMAND(AGRS)
ENDIF(expression)

# expression
IF(var)                                                                                                                              # 非空,0,N,NO,OFF,FALSE,NOTFOUNT或<VAR> NOTFOUND时，为真
IF(NOT var)                                                                                                                    # 与上述条件相反
IF(var1 AND var2)                                                                                                        # 逻辑与
IF(var1 OR var2)                                                                                                           # 逻辑或
IF(COMMAND cmd)                                                                                                     # 当给定的cmd确实是命令并可以调用是为真
IF(EXISTS dir)                                                                                                                #目录名存在
IF(EXISTS file)                                                                                                               # 文件名存在
IF(IS_DIRECTORY dirname)                                                                                    # 当dirname是目录
IF(file1 IS_NEWER_THAN file2)                                                                             # 当file1比file2新,为真
IF(variable MATCHES regex)                                                                                   # 符合正则

#循环
WHILE(condition)
    COMMAND(ARGS)
    //...
ENDWHILE(condition)


#******************************** CMake的常用指令及变量 **************************#
ADD_DEFINITIONS(-DXXX)                                                                                         # 为源文件的编译添加由-D引入的宏定义。
OPTION(<variable> "description [initial value]")                                             # 提供用户可以选择的选项

# 伪代码：为了说明生成自定义的命令
ADD_CUSTOM_COMMAND(TARGET ${CV_ADVANCE_NAME}                       # 给工程添加一条自定义的构建规则
    PRE_BUILD
    COMMAND "伪代码 find_package std_msg"    
)
# 为了说明导入生成自定义构建的命令
ADD_CUSTOM_TARGET(CV_ADVANCE ALL
    DEPEENDS ${CV_ADVANCE_NAME}                                                                     # 依赖ADD_CUSTOM_COMMAND输出的package包
    COMMENT "ros package std_msg"
)

# 用于解决链接时依赖的问题
ADD_DEPENDENCIES()

# 用于定义安装规则，安装的内容可以包括目标二进制、动态库、静态库以及文件、目录、脚本等
INSTALL(
    TARGETS myrun mylib mystaticlib
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION libstatic
)

# 设置include文件查找的目录，具体包含头文件应用形式，安装位置等
TARGET_INCLUDE_DIRECTORIES(<target>[SYSTEM][BEFORE]
                                                                    <INTERFACE|PUBLIC|PRIVATE>[items])

# 设置目标的一些属性来改变它们构建的方式
SET_TARGET_PROPERTIES(target1 target2 ...
                                                        PROPERTIES prop1 valu1 prop2 value2)

# 用来控制makefile是否构建test目标
ENABLE_TESTING()

# 一般需要和enable_testing()配合使用
# 生成makefile后可用make test执行测试
ADD_TEST(testname Exename arg1 arg2)

#******************************** CMake基本常用变量 ****************************#
CMAKE_INSTALL_PREFIX # 构建install的路径
$ENV{HOME}  # HOME环境下的目录路径
PROJECT_NAME # 工程名变量
<PKG>_INCLUDE_DIR # 导入包头文件全路径
<PKG>_LIBRARIES # 导入库文件的全路径
PROJECT_SOURCE_DIR # 构建工程的全路径
CMAKE_VERSION # CMake的版本号
CMAKE_SOURCE_DIR # 源码树的顶层路径



