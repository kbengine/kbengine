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
		public int port = @{KBE_LOGIN_PORT};
		
		// 客户端类型
		// Reference: http://www.kbengine.org/docs/programming/clientsdkprogramming.html, client types
		public KBEngineApp.CLIENT_TYPE clientType = KBEngineApp.CLIENT_TYPE.CLIENT_TYPE_MINI;

        //加密通信类型
        public KBEngineApp.NETWORK_ENCRYPT_TYPE networkEncryptType = KBEngineApp.NETWORK_ENCRYPT_TYPE.ENCRYPT_TYPE_NONE;

        // Allow synchronization role position information to the server
        // 是否开启自动同步玩家信息到服务端，信息包括位置与方向，毫秒
        // 非高实时类游戏不需要开放这个选项
        public int syncPlayerMS = 100;

		// 是否使用别名机制
		// 这个参数的选择必须与kbengine_defs.xml::cellapp/aliasEntityID的参数保持一致
		public bool useAliasEntityID = @{KBE_USE_ALIAS_ENTITYID};

        // 在Entity初始化时是否触发属性的set_*事件(callPropertysSetMethods)
        public bool isOnInitCallPropertysSetMethods = true;
        
		// 发送缓冲大小
		public MessageLengthEx TCP_SEND_BUFFER_MAX = NetworkInterfaceBase.TCP_PACKET_MAX;
		public MessageLengthEx UDP_SEND_BUFFER_MAX = 128;

		// 接收缓冲区大小
		public MessageLengthEx TCP_RECV_BUFFER_MAX = NetworkInterfaceBase.TCP_PACKET_MAX;
		public MessageLengthEx UDP_RECV_BUFFER_MAX = 128;

		// 是否多线程启动
		public bool isMultiThreads = false;

		// 只在多线程模式启用
		// 线程主循环处理频率
		public int threadUpdateHZ = @{KBE_UPDATEHZ};

		// 强制禁用UDP通讯
		public bool forceDisableUDP = false;

		// 心跳频率（tick数）
		public int serverHeartbeatTick = 15;

		public int getTCPRecvBufferSize()
		{
			return (int)TCP_RECV_BUFFER_MAX;
		}

		public int getTCPSendBufferSize()
		{
			return (int)TCP_SEND_BUFFER_MAX;
		}

		public int getUDPRecvBufferSize()
		{
			return (int)UDP_RECV_BUFFER_MAX;
		}

		public int getUDPSendBufferSize()
		{
			return (int)UDP_SEND_BUFFER_MAX;
		}
    }

} 
