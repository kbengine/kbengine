###v0.9.18

	2017/8/6

	新增与改善：
		防止Linux中用户自定义的未知信号造成的信号转信号名识别出错。
		调整部分调试信息。
		更新API文档。


	BUG修正：
		修正启动期被第三方库取消SIGPIPE监听的问题。（#498）
		修正controlledBy功能初始化时获取mailbox时没有减引用。（#499）



###v0.9.17

	2017/7/8

	新增与改善：
		getClientDatas支持获得注册时传入的datas。(#493)
		executeRawDatabaseCommand的回调接口增加参数insertID（回调如：def sqlcallback(result, rows, insertid, error)）。
		完善bots，使其支持controlledBy机制。
		WebConsole增加当前用户的UID显示，便于识别当前用户环境。
		更新API文档。


	BUG修正：
		修正一处错误日志信息。



###v0.9.16

	2017/6/21

	新增与改善：
		调整安全关服时实体销毁为分批销毁。
		添加API: KBEngine.kbassert，用于脚本中断底层，可以通过Core看堆栈信息。
		优化script::Map，部分操作直接使用原生的Python处理。
		更新API文档。


	BUG修正：
		修正FixedArray、FixedDict的一些操作导致内存泄漏或者异常问题。
		修正线程池可能拥有任务时因为异步问题导致没有立即执行任务却睡眠的问题。
		修正实体处于ghost状态时被调用cell的暴露客户端的方法时对方进程收到请求显示消息参数错误问题。



###v0.9.15

	2017/6/12

	新增与改善：
		对loginapp部分功能在Linux使用select替换为poll，避免在Linux上产生的一些问题。（#489）
		增加配置选项和实体属性entity.volatileInfo.optimized，可以取消实体位置同步优化，任何情况都同步Y，在房间内上下楼层型副本寻路可能有用。（#491）
		增加写日志的异常处理，当磁盘被写满时会抛出IOException，防止应用程序意外退出。
		简化跨进程传送流程，增加异步操作时的安全处理机制。
		更新API文档。


	BUG修正：
		解决某些时候加载持久化的数组顺序不对的问题。



###v0.9.14

	2017/6/6

	新增与改善：
		cellapp增加支持registerReadFileDescriptor之类的API接口。
		增加配置选项解决在端口映射环境强制暴露公网IP地址提供客户端登陆时，机器人程序不能直接走内网登陆问题。（#478）
		log4cxx_properties日志配置文件可以在子项目中重写，避免多个不同子项目需要改动底层默认格式引起冲突。（#479）
		当进程与logger异常断开连接后，一部分缓存待发送的log也将输出到日志文件中，避免遗漏关键错误日志。
		导航模块同时支持unity插件导出的navmesh文件格式和recastnavigation原生的格式。
		GUIConsole工具支持端口映射环境连接腾讯云、阿里云等环境的局域网内部KBE进程。
		baseapp和dbmgr脚本增加onReadyForShutDown回调，允许控制进程安全退出的时机，具体参考API手册。
		多组服务器允许共用一个账号系统。（#413）
		更新API文档。


	BUG修正：
		解决部署进程数量特别多时， 由于端口竞争造成部分进程没有能成功提交自己的身份到machine问题。
		修正邮箱认证输入错误邮箱没有错误提示（#480）
		修正设置新密码，服务器回调错误（#481）
		修正绑定邮箱请求服务器发送的email中地址是localhost（#483） 
		修正x64版本下WebConsole的SpaceView功能异常的问题。
		修正修改cellData后并没有标脏，导致主动调用writeToDB并没有及时存档
		修正一些出错的情况下Channel未释放的问题。
		修正loginapp脚本接口的错别字，onReuqestLogin改为onRequestLogin。



###v0.9.13

	2017/4/22

	新增与改善：
		解决safe_kill后日志会报Abnormal误报问题。(#466)  
		更新API文档。


	BUG修正：
		修正WebConsole在Python2.x下的异常错误
		修正部分平台上dbmgr连接interfaces出错的问题
		修正assets/start_bots.sh启动环境设置错误问题
		修正当没有cell的entity其base部分存存储时无法正确回调onWriteToDB(self, cellData)脚本的问题 



###v0.9.12

	2017/3/21

	新增与改善：
		Vector传输优化， 不需要记录size。(#430) 
		增加支持在服务器调用玩家的disconnect功能断开客户端连接(entity.disconnect())
		实体移动updatable和witness的updatable之间没有优先级，导致位置可能没有及时在同一帧更新(#461)
		更新API文档。


	BUG修正：
		修正entitiesInAOI在极端情况产生异常。(#459)
		修正配置中databaseInterfaces多个接口中存在注释时读取错误的问题 



###v0.9.11

	2017/2/25

	新增与改善：
		防止def中属性和方法重名。(#449) 
		对坐标系统进行优化
		installer.py脚本linux安装完善(#451)
		entityID资源每次申请的段大小支持可配置（increasing_range）(#453)
		移除引擎内部对Entity.topSpeed添加的冗余值，这个冗余值会对使用者产生Entity.topSpeed机制无效或不稳定的困扰。
		更新API文档。


	BUG修正：
		修正将Account实体作为Player时断线重连重新获得控制权后客户端没有收到enterworld消息。(#454)
		修正websocket协议在一个包数据不全时解析出错的问题
		修正冻结账号后用户登陆返回的错误码并不是账号被冻结码
		修正被控制的对象移动速度不受Entity.topSpeed限制的问题（#433）
		修正FixedArray::insert在脚本层错误的只给了一个参数时导致错误，必须给入2个参数



###v0.9.10

	2017/1/24

	新增与改善：
		WebConsole支持在线查看游戏实时空间状态。 
		对坐标系统进行优化
		加强回调嵌套调用导致可能的意外的防护处理
		更新API文档。


	BUG修正：
		修正cellapp中， 在实体__init__中输出位置总是为(0,0,0)。(#447)
		修正使用Entity.entitiesInRange(5.0, None, None)这样的方式调用时，提示args(position) error!”错误的问题



###v0.9.9

	2017/1/4

	新增与改善：
		修改pickler还原数据失败时的输出日志，使其输出完整的原始数据，以方便出错时定位错误位置。
		更新API文档。


	BUG修正：
		修正对象池瘦身时间判断错误    
		修正在包异常时没有做包缓存清理



###v0.9.8

	2016/12/24

	新增与改善：
		对一些接口做外部通道攻击防护。 
		更新API文档。


	BUG修正：
		解决webconsole获取space相关信息失败的问题   
		解决某些情况下账号entity已创建，但是因数据库操作失败本因销毁掉已创建的账号entity而未销毁的问题



###v0.9.7

	2016/12/1

	新增与改善：
		对装入泄漏监视器的实体如果长时间没有销毁则定期输出日志警告(#431)
		增加：WebConsole増加服务器组件状态动态显示功能。 
		创建实体失败增加更具体的错误提示 
		更新API文档。


	BUG修正：
		修正KBEngine.lookUpBaseByDBID第二个参数如果给入是非DBID， 错误提示有误的问题。  
		重登陆可能造成pAOITrigger_不在坐标系统中，而进行重安装时crash了 
		修正def中给一个int类型的属性增加DatabaseLength设定时，sql语句出错的问题(#435)



###v0.9.6

	2016/11/11

	新增与改善：
		防止createBaseRemotelyFromDBID、createBaseRemotely接口请求创建时baseapp还未注册到baseappmgr而丢失请求(#429)
		更新API文档。


	BUG修正：
		防止登陆顶号onLogOnAttempt中销毁实体造成底层后续流程访问对象出错的问题 。 



###v0.9.5

	2016/11/8

	新增与改善：
		网络层一点小优化(#420)。 
		WebConsole增加Watcher查看功能。
		支持使用GCC 6.2.1及以上编译器版本编译引擎(#425)。
		更新API文档。


	BUG修正：
		修正多线程下DebugHelper::onMessage中检查日志超量做清理clearBufferedLog时可能有线程竞争(#426)
		修正Linux上第一次安装编译源码第三方库出错后就无法再次编译通过问题(#427)



###v0.9.4

	2016/10/19

	新增与改善：
		数据库kbe_entitylog表新增加logger字段， 表示由哪个dbmgr记录。 


	BUG修正：
		onMove中移动自己后销毁自己有概率crash（#414）。
		controlledBy设置为None，玩家周围没有怪服务器无法移动玩家（#416）



###v0.9.3

	2016/10/8

	新增与改善：
		增加对转发消息的具体消息追踪支持。
		降低向客户端send次数。(#389) 
		cluster_controller.py工具对单个进程关闭支持。 (#390)
		KBEngine.SERVER_ERR_NEED_CHECK_PASSWORD改名为KBEngine.SERVER_ERR_LOCAL_PROCESSING
		WebConsole改进，増加Machine及相关组件自动更新缓存功能，以让减少查询页卡顿现象。
		machine支持杀死某个进程，例如某些时候进程卡死webconsole无法关闭。
		提供给外部服务查询服务器负载值内存实体数量等功能。(#403)
		对于实体运动可能需要加减速的支持, 增加API接口accelerate(#371)。
		增加新配置选项tickSentBytes，一个tick内发送的字节数溢出限制，具体见kbengine_defs.xml。(#411)
		给base增加获取数据库接口名的办法, Base.databaseInterfaceName。(#412)
		API文档更新


	BUG修正：
		修正pytickprofile命令未结束时关闭控制台有可能导致服务器崩溃的问题 
		修正机器人偶尔登陆出现digest not match.（#395）。
		修正启动不同项目后，数据库相同时数据库表其中字段被同步成sm_autoLoad_2(#406)
		修正一处范围触发器极端情况下漏掉实体进出事件问题
		其他一些修正（#391、#392、#388、#410）



###v0.9.0

	2016/8/12

	新增与改善：
		Windows下支持启动参数以后台方式启动服务器（增加--hide=参数，--hide=1隐藏窗口，#359）。
		onDestroy中isDestroyed_ = true应该在脚本回调之前设置，否则可能在期间导致脚本再调用某些死亡后不可用功能造成问题。 
		PyMemoryStream增加rpos和wpos与fill脚本方法。 
		addYawRotator使用后客户端表现混乱旋转问题 (#366)
		客户端心跳回调实现 (#369)
		增加新的API支持（createBaseRemotely 、createBaseRemotelyFromDBID）#372。
		脚本入口模块名称统一调整为kbemain，避免因名字重合混淆一些概念，具体见kbengine_defs.xml。
		优化了坐标管理系统。
		API文档更新


	BUG修正：
		修正某些情况销毁space时造成crash的问题。 
		修正CoordinateSystem::removeReal没有释放内存（#373）。
		修正setAoiRadius不能动态的改变AOI看到的内容问题（#375）
		修正使用VC启动cellapp就会crash的问题（#376）
		其他一些修正（#360、#370、#374、#378、#381、#377）



###v0.8.10

	2016/6/27

	新增与改善：
		大幅提升cellapp性能，以及整体性能(#333)
		API文档更新
		防止def中属性没有写Flags、Type标签，对此情况返回错误并给出错误警告。
		防止def中utype设置重复的值(#355)
		防止脚本模块名字与Python原生模块名冲突(#358)
		controllerBy机制实现，该机制允许不同的权限控制其他或者自己的实体(#224)
		KBEngine.charge系列函数不再使用KBEngine.MemoryStream，统一使用Bytes 
		结构中字段支持DatabaseLength(#354)
		增加加密包的调试功能，将trace_packet打开，并将其中屏蔽的Encrypted::packets消息去掉即可输出。
		packetAlwaysContainLength支持(#351)
		防止在各种脚本回调中销毁自己导致crash (#348)
		telnet控制台増加“:pytickprofile”命令。以tick为单位输出每一帧的脚本执行消耗数据。 
		新增针对具体地址池进行组网，解决跨网段不能广播导致无法启动服务器问题(#343)
		所有ghost状态下，不允许其调用allClients、otherClients、clientEntity 
		新增webconsole第一版
		addSpaceGeometryMapping指向的目录如果不存在应该给出错误提示(#350)


	BUG修正：
		修正一定概率下，实体销毁后witnesses列表不为空的问题 
		修正固定字典key写成非字符串导致crash，例如：self.characters[1] = x 
		修正 实体新增入库属性不会按照配置设置默认值(#337) 
		修正对NPC调用entitiesInAOI()崩溃的问题



###v0.8.0

	2016/1/30

	新增与改善：
		多数据库横向扩展支持（#264）
		VS2015编译支持（#292）
		Windows X64位编译支持（#282）
		OPENSSL 升级到1.0.2e
		API文档更新
		Recastnavigation 更新至最新版本
		cluster_controller.py支持远程启动进程
		错误信息在linux上也打印到控制台， 有助于启动时及时发现错误（#280）
		KBEngine.Blob改名为KBEngine.MemoryStream
		规范化API命名，所有脚本主动调用的称为引擎API函数，所有由引擎通知脚本被动接受调用的称为引擎回调（requestCreateAccount、requestAccountLogin、requestCharge分别改名为onRequestCreateAccount、onReques、onRequestCharge）
		logger脚本支持
		entitiesInRange优化（#298）
		base上实体调用createInNewSpace之后， 在cell上也应该走一次enterspace等流程。
		Entity.volatileInfo脚本可改支持

	BUG修正：
		修正邮件绑定后不能使用邮件登录问题
		其他修正：#289、#290



###v0.7.0

	2015/11/25

	新增与改善：
		邮件重置密码完善
		更新API文档
		抽象化interfaces，interfaces使用python进行扩展
		loginapp支持脚本了,可扩展脚本做类似登陆排队功能和控制账号的登陆等行为
		deregisterFileDescriptor改名为deregisterReadFileDescriptor
		proxy在destroy后及时通知客户端被服务器踢出
		支持让某个baseapp、cellapp不参与负载均衡（KBEngine.setAppFlags、KBEngine.getAppFlags）
		增加新的API：Entity.getRandomPoints用于随机获取目的坐标点周围navigate可到达的指定数量的坐标点（可用于NPC随机移动，掉落物品坐标计算等）。
		
	BUG修正：
		修正loginapp的http回调端口返回页面时乱码现象
		修正:一个entity属性设成BOOL（也就是UINT8），然后退出服务器，改成了INT8，重起服务器以后，数据表不会变bug(#263)
		修正在服务器上不存在某实体的时候，客户端请求实体方法可能造成crash
		防止APP在退出时有日志没有同步完（同步到logger）。



###v0.6.21

	2015/10/26

	新增与改善：
		将原有的CLIENYT_TYPE_PC拆分成CLIENYT_TYPE_WIN、CLIENYT_TYPE_LlINUX、CLIENYT_TYPE_MAC
		Entitydef的摘要检查不再与客户端类型绑定在一起，如果客户端提交了摘要则检查，没有提交则客户端对自己的协议正确性负责（如果客户端严格从服务器远程导入协议，理论上不会有问题）
		标准化一些协议名称



###v0.6.20

	2015/10/23

	新增与改善：
		增加新的API：Entity.addYawRotator
		更新API文档
		vs2010项目升级到vs2013
		实体容器类属性标脏机制(#259)
		addSpaceGeometryMapping参数调整可指定参数加载navmesh到某个layer下(#240)
		调整数据库查询接口，更好的支持不同形式的数据库扩展

	BUG修正：
		修正email激活邮件乱码问题
		修正指定FIXED_DICT类型存档字段不对的问题(#255)
		修正dbmgr多次与interfaces连接的问题
		修正moveToPoint的参数distance大于0时当实体距离目的地小于distance时实体会忘相反的地方行走



###v0.6.1

	2015/6/1

	新增与改善：
		数据库所有系统主键ID改为uint64类型
		增加服务端切换地图时回调onEnterSpace/onLeaveSpace
		当客户端请求获得服务端协议时，将属性flags也下发到客户端，用于属性类型的判断
		一些结构调整

	BUG修正：
		修正某种情况下实体销毁并未通知客户端销毁的问题
		修正某些情况下teleport后，客户端没有改变实体位置



###v0.6.0

	2015/5/3

	新增与改善：
		Websocket FRC6544协议通讯完善
		cluster_controller.py（集群控制）、installer.py（安装助手）工具完善
		assets默认不包含scripts/client目录，避免不必要的误会(移动到OGRE例子脚本目录中)
		更新API文档
		一些结构调整

	BUG修正：
		修正bots程序退出时crash问题
		修正在进程繁忙状态下有时不能很好的安全关服
		修正FixedDict引用未释放问题



###v0.5.0

	2015/4/18

	新增与改善：
		优化部分代码结构
		增加新的API（getClientDatas），用于在脚本中读取客户端登陆时所附带的数据
		增加API，getAoiRadius、getAoiHystArea
		统一向服务端同步方向的顺序是roll、pitch、yaw
		bots增加entryScriptFile配置 

	BUG修正：
		Cellapp上的Exposed方法，第一个参数得到的并非是调用者的ID
		修正BASE_AND_CLIENT类属性在客户端实体创建之后才同步过来
		修复客户端注册账号时上报的数据过长导致dbmgr出错的问题



###v0.4.20

	2015/3/13

	新增与改善：
		优化部分代码结构
		增加logger处理速度
		API文档更新

	BUG修正：
		修正windows下genUUID64可能不正确的问题
		修正bots在某些条件下导致日志缓存没有释放的问题



###v0.4.10

	2015/2/10

	新增与改善：

	    实体自动加载功能（用法见API手册，baseapp中writeToDB部分）
	    onBaseAppReady、onReadyForLogin回调函数参数调整，改为bool（是否为第一个启动的baseapp）
	    增加进程启动参数 --gus，详见：http://www.kbengine.org/cn/docs/startup_shutdown.html
	    删除进程启动参数--grouporder与--globalorder，该值由程序内部自动产生，通过环境变量在脚本中可以获得KBE_COMPONENTID、KBE_BOOTIDX_GLOBAL、KBE_BOOTIDX_GROUP
	    API文档完善

	BUG修正：

	    修正teleport后ghost消息错乱问题
	    修正teleport后pitch错误的问题



###v0.4.0

	2015/1/23

        新增与完善：

		网络模块优化（主要针对send以及一些结构方面）
		增加对TCP发送窗口的配置，以便对一些情况进行控制
		负载均衡模块调整（Cellapp在大量动态副本创建时能更好的均衡负载）
		消息跟踪模块调整
		
	BUG修正：
		
		修正EntityDef，可能在一些情况下def改变后MD5并未变化的问题
		修正Cellapp初始化时执行其他进程要求的任务产生的错误
		修正DBMgr如果宕了，interfaces与logger进程等自动关闭



###v0.3.0
	
	2014/12/30

	新增与完善：

	    完善API文档
	    GUIConsole增加log搜索与过滤等功能
	    完善cluster_controller了
	    完善了installer功能
	    billingsystem改名interfaces、kbmachine改名为machine、messagelog改名为logger
	    增加默认的项目资产目录"assets"，如果没有配置环境变量引擎将自动从根目录寻找该资产目录
	    原本的demo资产目录被迁移到独立的项目中，在具体的demo中使用git submodule获得
	    其他若干小完善

	BUG修正：

	    修正BASE_AND_CLIENT等类型被修改后没有同步到客户端的问题
	    修正CLIENT_TYPE_MOBILE类型要求判断entitydef的错误
	    修正实体跳转场景后ghost机制带来的消息错乱问题



###v0.2.15

	2014/12/6

	新增与改善：
	    脚本层字符串属性序列化到流优化， 可以减少一次内存拷贝
	    增强GUIConsole的探测功能
	    对部分代码结构进行调整

	BUG修正：
	    修正AOI极端情况下一些状态错乱的问题
	    globalorderid等应该使用int32类型目前int8， 这样会限制理论的进程数量
	    避免在目录下没有log4j.log时提示错误信息

