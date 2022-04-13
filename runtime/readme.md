### 1. 安装(win10)
* visual stdio 2017
* git rl_bwe分支源码到`rl_bwe\`文件夹下
```
git clone -b rl_bwe https://git.code.tencent.com/TX-HUST-2021-2022/AlphaRTC.git rl_bwe
```

* 安装编译工具并手动设置环境变量：
    - [depot-tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)
    - [ninja](https://blog.csdn.net/qq_20373723/article/details/84061647)

* 解压src压缩包，并将src目录下所有文件`src\*`解压到`rl_bwe\`目录下
### 2. 配置环境
**编译需和环境变量配置在同一个cmd进程中，否则需要重新配置**

打开**cmd**或**vs2017的开发人员命令提示符**输入下列环境变量（根据自身vs版本进行修改，在此以vs2017 community为例）
```
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

set vs2017_install=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community

set GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community

set GYP_GENERATORS=msvs-ninja,ninja

set DEPOT_TOOLS_WIN_TOOLCHAIN=0
```
* 编译: 在`rl_bwe\`目录下执行：

```
gn gen out\test
ninja -C .\out\test peerconnection_serverless
```

该命令会自动生成test目录

### 3. 运行
* 将`modules\third_party\onnxinfer\bin`下dll文件复制到`out\test`下
* 进入rl_bwe\runtime目录：`cd runtime`
* 修改 `receiver_pyinfer.py`和`sender_pyinfer.py`中主机ip地址
* 运行ppo_solution的`main.py`. 开启两个py进程一个是receiver一个是sender.
```
python main.py 'server_ip' server_port
```
```
python baseline-ppo/baseline/PPO/main.py '192.168.0.189' 1234
```
```
python baseline-ppo/baseline/PPO/main.py '192.168.0.189' 4321
```
等待两个进程都出现：`"Waiting for connection... Server IP: 192.168.0.189, PORT: 4321"`字样再执行下一步

* 在`runtime\`目录下，运行:
```
可执行文件.exe configfile.json server_port
```
先执行receiver
```
..\out\test\peerconnection_serverless.exe .\receiver_pyinfer.json 1234
```
后执行sender
```
..\out\test\peerconnection_serverless.exe .\sender_pyinfer.json 4321
```