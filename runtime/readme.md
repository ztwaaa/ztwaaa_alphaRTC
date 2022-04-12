### 1. 安装(win10)
* visual stdio 2017
* 将`src\*`所有文件解压到`rl_bwe\`目录下
* 配置环境：
```
"C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"

set vs2017_install=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community

set GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community

set GYP_GENERATORS=msvs-ninja,ninja

DEPOT_TOOLS_WIN_TOOLCHAIN=0
```
* 编译：在`rl_bwe\`目录下执行：
```
gn gen out/test
ninja -C .\out\test peerconnection_serverless
```

### 2. 运行
* 将`modules\third_party\onnxinfer\bin`下dll文件复制到`out\test`下
* 进入rl_bwe\runtime目录：`cd runime`
* 修改 `receiver_pyinfer.py`和`sender_pyinfer.py`中主机ip地址
* 运行ppo_solution, `main.py`. 开启两个py进程一个是receiver一个是sender.
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

* 在`runime\`目录下，运行:
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