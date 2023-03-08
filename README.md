# CMS61850

## 一、前言	

​	cms61850又称为国产（中国）61850，取代国际版61850而生，架构较国际版相比，更为简洁，性能有较大提升，摒弃了mms，直接基于tcp。同时结合国密，可做到安全通讯，不久的将来可能会大面积铺开。61850可分为服务端与客户端，本项目处于初始阶段，目前仅开源了服务端的部分代码。后续会逐步更新。用户基于此框架，可迅速开发部署cms61850相关服务，减短开发周期



## 二、使用

​	本项目基于cmake，使用前用户应当安装cmake，若提示版本过低，请升级。目前代码仅支持linux系统，支持x86及aarch64架构，有新平台需要者，可联系作者进行适配编译。

### 2.1、编译

在任意目录处，clone代码

`git clone https://github.com/LinuxZQ93/CMS61850.git`

切换至项目目录进行编译，内置了build.sh脚本辅助编译（可根据需要自我修改）

`cd ./CMS61850`

`./build.sh`

默认编译的是x86架构，若是编译aarch64架构，则用./build.sh aarch64

### 2.2、运行

编译成功后，可切换至项目./Run目录，直接执行run.sh即可。由于程序做了证书加密处理，运行需要license认证，可联系作者获取license

程序运行可根据自己需要进行配置修改，配置文件在./config/CMS61850/cms.json，内容如下

```json
{
	"CMS61850" : 
	{
		"associate" :   ---->配置协商参数的
		{
			"apduSize" : 65535, ---->apdu大小
			"asduSize" : 65531,  ---->asdu大小
			"protocol" : 513,  ---->协议版本，仅支持513
			"safe" :   ---->安全参数
			{
				"application" :   ---->应用层安全
				{
					"enable" : true,  ---->是否使能
					"rootCA" : "./gmssl/sm2.ca.pem",   ---->根域名
					"serverCert" : "./gmssl/sm2.PNC.sig.crt.pem",  ---->服务端证书
					"serverKey" : "./gmssl/sm2.PNC.sig.key.pem"  ---->客户端证书
				}
			},
			"timeDiff" : 600  ---->校验时间差值
		},
		"connectNum" : 16,  ---->支持最大的客户端连接数
		"errorNum" : 4,   ---->最大支持错误的交互次数
		"icdPath" : "./config/CMS61850/IEC61850.icd",  ---->icd文件目录
		"port" : 8102,  ---->端口
		"transport" :   ---->tls加密参数，暂不支持
		{
			"enable" : false,
			"encCert" : "./gmssl/encCert.pem",
			"encPass" : "./gmssl/encPass.pem",
			"rootCA" : "./gmssl/rootca.pem",
			"sigCert" : "./gmssl/sigCert.pem",
			"sigPass" : "./gmssl/sigPass.pem"
		}
	}
}
```



## 三、结语

目前代码支持如下功能

| 服务码 | 服务接口                |
| ------ | ----------------------- |
| 1      | Associate               |
| 2      | Abort                   |
| 3      | Release                 |
| 80     | GetServerDirectory      |
| 81     | GetLogicDeviceDirectory |
| 82     | GetLogicNodeDirectory   |
| 83     | GetAllDataValues        |
| 155    | GetAllDataDefinition    |
| 48     | GetDataValues           |
| 49     | SetDataValues           |
| 50     | GetDataDirectory        |
| 51     | GetDataDefinition       |
| 57     | GetDataSetDirectory     |
| 58     | GetDataSetValues        |
| 89     | GetSGCBValues           |
| 91     | GetBRCBValues           |
| 93     | GetURCBValues           |
| 94     | SetURCBValues           |
| 153    | Test                    |
| 154    | AssociateNegotiate      |

开源程度

| 名称      | 功能                            | 是否开源 |
| --------- | ------------------------------- | -------- |
| framework | 程序主体框架，各类组件运行中心  | 否       |
| aper      | APER编码                        | 否       |
| cms61850  | cms61850核心业务实现            | 是       |
| gmssl     | 国密sm2/3/4实现                 | 否       |
| thirdLib  | 其余三方库，比如xml，json解析等 | 可开源   |

**其中aper基于asn1c进行开发，gmssl基于也是基于gmssl的开源项目开发。用户也可以自行基于此进行二次开发**

**注：本代码仅供参考学习，未经作者允许，不得擅自用于各类商业项目，并且不能进行随意传播**

61850相关问题可加微信HardAndBetter或者入qq群586166104沟通，亦可直接email沟通linuxzq93@163.com
