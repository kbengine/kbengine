namespace KBEngine
{
	using System; 
	
	using MessageLengthEx = System.UInt32;
	
	/*
		初始化KBEngine的参数类
	*/
    public class KBEngineArgs 
    {
    	// 登录ip和端口
		public string ip = "127.0.0.1";
		public int port = 20013;
		
		// 客户端类型
		// Reference: http://www.kbengine.org/docs/programming/clientsdkprogramming.html, client types
		public KBEngineApp.CLIENT_TYPE clientType = KBEngineApp.CLIENT_TYPE.CLIENT_TYPE_MINI;
		
		// 持久化插件信息， 例如：从服务端导入的协议可以持久化到本地，下次登录版本不发生改变
		// 可以直接从本地加载来提供登录速度
		public string persistentDataPath = "";

		// Allow synchronization role position information to the server
		// 是否开启自动同步玩家信息到服务端，信息包括位置与方向
		// 非高实时类游戏不需要开放这个选项
		public bool syncPlayer = true;

		// 是否使用别名机制
		// 这个参数的选择必须与kbengine_defs.xml::cellapp/aliasEntityID的参数保持一致
		public bool useAliasEntityID = true;

        // 在Entity初始化时是否触发属性的set_*事件(callPropertysSetMethods)
        public bool isOnInitCallPropertysSetMethods = true;
        
		// 发送缓冲大小
		public MessageLengthEx SEND_BUFFER_MAX = NetworkInterface.TCP_PACKET_MAX;

		// 接收缓冲区大小
		public MessageLengthEx RECV_BUFFER_MAX = NetworkInterface.TCP_PACKET_MAX;

		// 是否多线程启动
		public bool isMultiThreads = false;

		// 只在多线程模式启用
		// 线程主循环处理频率
		public int threadUpdateHZ = 10;

		public int getRecvBufferSize()
		{
			return (int)RECV_BUFFER_MAX;
		}

		public int getSendBufferSize()
		{
			return (int)SEND_BUFFER_MAX;
		}
    }

} 
