# 1 准备

- 系统要求
  - 两台Win7及以上64位系统的主机（推荐Win10，Win7下问题多）。
  - 内存至少8G，当然越大越好。
  - 至少50G磁盘空间（NTFS格式），不能是FAT32，因为会生成大于4G的文件。
  

- 安装visual stdio 2019/2017

  要求VS2017 (>=15.7.2)或以上。这里我用的是社区版VS2019。安装VS2019时选择自定义安装，必须勾选如下几项：

  - **Desktop development with C++组件中10.0.20348.0或以上的Win10 SDK**（如果没看到该版本，去左侧Individual components那里勾选）
  - **Desktop development with C++组件中MFC以及ATL**这两项

  安装完VS2019后，须安装SDK调试工具：

  1. 打开控制面板->程序与功能，找到刚才安装的最新Windows Software Development Kit，鼠标右键->change。

  2. **勾选Debugging Tools For Windows，然后点击change**。

- git AlphaRTC分支源码到`AlphaRTC\`文件夹下

  ```
  git clone -b rl_bwe https://git.code.tencent.com/TX-HUST-2021-2022/AlphaRTC.git AlphaRTC
  ```

- 将`AlphaRTC-src/src.rar`里`src`文件夹下所有文件解压到 `AlphaRTC\`

- RL-server（MetaCC）

  clone MetaCC到AlphaRTC同一文件夹

  ```
  git clone -b baseline-ppo-stable https://git.code.tencent.com/WaterHyacinthInNANHU/METAcc MetaCC
  ```

- 安装编译工具

  - [depot-tools](https://commondatastorage.googleapis.com/chrome-infra-docs/flat/depot_tools/docs/html/depot_tools_tutorial.html#_setting_up)

    - Download the *depot_tools* [bundle](https://storage.googleapis.com/chrome-infra/depot_tools.zip) and extract it somewhere.

      | Warning | **DO NOT** use drag-n-drop or copy-n-paste extract from Explorer, this will not extract the hidden “.git” folder which is necessary for *depot_tools* to autoupdate itself. You can use “Extract all…” from the context menu though. |
      | ------- | ------------------------------------------------------------ |

      Add *depot_tools* to the *front* of your PATH (must be ahead of any installs of Python). Assuming you unzipped the bundle to `C:\workspace\depot_tools`:

      - With Administrator access:

        **Control Panel → System and Security → System → Advanced system settings**Modify the PATH system variable to include `C:\workspace\depot_tools`.

      - Without Administrator access:

        **Control Panel → User Accounts → User Accounts → Change my environment variables**Add a PATH user variable: `C:\workspace\depot_tools;%PATH%`.

      From a `cmd.exe` shell, run the command `gclient` (without arguments). On first run, gclient will install all the Windows-specific bits needed to work with the code, including msysgit and python.

      **Note**

      - If timeout error occurs when you run gclient, set proxy as follows:

        ```
        set http_proxy=127.0.0.1:<port>
        set https_proxy=127.0.0.1:<port>
        ```

      - If you run gclient from a non-cmd shell (e.g., cygwin, PowerShell), it may appear to run properly, but msysgit, python, and other tools may not get installed correctly.
      
      - If you see strange errors with the file system on the first run of gclient, you may want to [disable Windows Indexing](http://tortoisesvn.tigris.org/faq.html#cantmove2).
      
      - After running gclient open a command prompt and type `where python` and confirm that the depot_tools python.bat comes ahead of any copies of python.exe. Failing to ensure this can lead to overbuilding when using gn - see [crbug.com/611087](https://bugs.chromium.org/p/chromium/issues/detail?id=611087).
      
      - check if you are done
      
        ```
        gn help
        ```
  
  - ninja

    - ```
      git clone https://github.com/ninja-build/ninja.git && cd ninja
      ```
  
      ```
      git checkout release
      ```
  
      ```
      "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvarsall.bat" x64
      ```
      
        测试是否安装成功
      
        ```
      ninja --version
        ```

# 2 编译

**编译需和环境变量配置在同一个cmd进程中（即接下来的所有操作都在同一个cmd窗口完成），否则需要重新配置**

打开**cmd**或**vs2019的开发人员命令提示符**输入下列环境变量（根据自身vs版本和安装路径进行修改以下代码块，在此以vs2019 community为例）复制以下代码块直接粘贴到cmd

```
"C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
set vs2019_install=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
set GYP_MSVS_OVERRIDE_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
set GYP_GENERATORS=msvs-ninja,ninja
set DEPOT_TOOLS_WIN_TOOLCHAIN=0


```

- 编译: 在`AlphaRTC\`目录下执行：

```
gn gen out\test
```

该命令会自动生成test目录

**编译无GUI版本程序**

运行以下命令编译：

```
ninja -C .\out\test peerconnection_serverless
```

编译完成后显示：

```
rconnection_serverless
ninja: Entering directory `.\out\test'
[3110/3110] LINK peerconnection_serverless.exe peerconnection_serverless.exe.pdb
```

**编译有GUI版本程序**

运行以下命令编译：

```
ninja -C .\out\test peerconnection_serverless_win_gui
```

编译完成后显示：

```
rconnection_serverless
ninja: Entering directory `.\out\test'
[3110/3110] LINK peerconnection_serverless.exe peerconnection_serverless_win_gui.exe.pdb
```

# 3 运行

> 注：有GUI版本和无GUI版本运行命令是一样的，只需修改程序名 `peerconnection_serverless.exe` 和 `peerconnection_serverless_win_gui.exe`即可

## 3.1 RL server

RL server用于响应AlphaRTC的带宽估计请求，负责RL网络推理和记录运行状态信息。

在RL server上启动MetaCC 进程：

- 修改 `AlphaRTC\runtimereceiver_pyinfer.json`和`AlphaRTC\runtimesender_pyinfer.json`中主机ip地址为RL server地址
- 运行ppo_solution的`main.py`. 开启两个MetaCC进程：一个服务receiver一个服务sender.
- 参数是：1. socket ip（可以是127.0.0.1，但如果要被所在网络其它机器访问则设置为本机在该网络的ip） 2. 端口

```
python main.py 'server_ip' server_port
python baseline-ppo/baseline/PPO/main.py 192.168.0.189 1234
python baseline-ppo/baseline/PPO/main.py 192.168.0.189 4321
```

等待两个进程都出现：`"Waiting for connection... Server IP: 192.168.0.189, PORT: ..."`

## 3.2 AlphaRTC

- 首次运行：将`modules\third_party\onnxinfer\bin`下dll文件复制到`out\test`下

- 修改配置文件，参考附录

- 在`AlphaRTC\`目录下：

  先执行receiver

  ```
  out\test\peerconnection_serverless.exe runtime\receiver_pyinfer.json
  ```

  后执行sender

  ```
  out\test\peerconnection_serverless.exe runtime\sender_pyinfer.json
  ```

- 在我们实验室后来改的版本中，建议先使用`AlphaRTC\runtime\ssender.json`和`AlphaRTC\runtime\rreceiver.json`搭配serverless.exe先试试。
- 若使用GUI版本，则允许`peerconnection_serverless_win_gui.exe`
> sender 和 receiver 没有主从之分，它们互相发送并接收视频，地位相同。只是在会话开始的时候需要sender主动向receiver发送请求来建立连接，因此只需告知sender receiver的ip即可，receiver无需被事先告知sender ip。

## 3.3 Dummy server
- 运行`AlphaRTC\dummy_server\METAcc\runtime\main.py` 
```
python main.py <ip port> 例如 python main.py 127.0.0.1 6666
```
- 在命令行即可看到AlphaRTC输出的参数
# 参考

1. [Windows平台WebRTC编译（持续更新） - 剑痴乎 (jianchihu.net)](https://blog.jianchihu.net/windows-webrtc-build.html)
2. https://git.code.tencent.com/TX-HUST-2021-2022/AlphaRTC/tree/rl_bwe/runtime

# 附录

## 配置文件说明

`runtime\sender_pyinfer.json`

```json
{
    "serverless_connection": {
        "autoclose": 1000, 多少秒后自动关闭会话
        "sender": {
            "enabled": true,
            "dest_ip": "10.0.0.2", receiver ip
            "dest_port": 8000	receiver port
        },
        "receiver": {
            "enabled": false
        }
    },
    
    "bwe_algo": "gcc", 注意这里，如果使用gcc是不会给socket接口发送信息的，需要使用`rlcc`
    "socket_config": {
        "socket_server_ip": "192.168.31.250", RL server ip
        "socket_server_port": 1234	RL server port
    },

    "bwe_feedback_duration": 200,
    "video_source": {
        "video_disabled": {
            "enabled": false
        },
        "webcam": {
            "enabled": false 是否启用摄像头
        },
        "video_file": { 离线视频源配置
            "enabled": true,
            "height": 240, 视频源宽
            "width": 320, 视频源高
            "fps": 10, 视频源fps
            "file_path": "testmedia/test.yuv" 视频源路径
        }
    },
    "audio_source": {
        "microphone": {
            "enabled": false 是否启用麦克风
        },
        "audio_file": { 离线音源配置
            "enabled": true,
            "file_path": "testmedia/test.wav" 音源路径
        }
    },
    "save_to_file": {
        "enabled": false
    },
    "logging": {
        "enabled": false
    }
}
```

`runtime\receiver_pyinfer.json`

```json
{
    "serverless_connection": {
        "autoclose": 1500, 多少秒后自动关闭会话
        "sender": {
            "enabled": false
        },
        "receiver": {
            "enabled": true,
            "listening_ip": "10.0.0.3", 本机在目标网络中的ip
            "listening_port": 8000 本机会话端口
        }
    },
    
    "bwe_algo": "gcc",  注意这里，如果使用gcc是不会给socket接口发送信息的，需要使用`rlcc`
    "socket_config": {
        "socket_server_ip": "127.0.0.1", RL server ip（这里server可以直接设置使用GCC，一般不关心receiver->sender的视频流）
        "socket_server_port": 1234 RL server port
    },

    "bwe_feedback_duration": 200,
    "video_source": {
        "video_disabled": {
            "enabled": false
        },
        "webcam": {
            "enabled": false
        },
        "video_file": { 离线视频源配置
            "enabled": true,
            "height": 240, 视频源宽
            "width": 320, 视频源高
            "fps": 10, 视频源fps
            "file_path": "testmedia/test.yuv" 视频源路径
        }
    },
    "audio_source": {
        "microphone": {
            "enabled": false 是否启用麦克风
        },
        "audio_file": { 离线音源配置
            "enabled": true,
            "file_path": "testmedia/test.wav" 音源路径
        }
    },
    "save_to_file": { 会话视频（sender->receiver的视频）保存配置（sender不读取此项，暂时只有receiver能保存视频）
        "enabled": false, 是否启用会话视频保存
        "audio": {
            "file_path": "outaudio.wav" 音频存储路径
        },
        "video": {
            "width": 640, 会话视频宽
            "height": 480, 会话视频高
            "fps": 30, 会话视频fps
            "file_path": "outvideo.yuv" 会话视频存储路径
        }
    },
    "logging": {
        "enabled": true,
        "log_output_path": "webrtc.log"
    }
}
```

