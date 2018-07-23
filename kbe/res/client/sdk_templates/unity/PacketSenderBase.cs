namespace KBEngine
{
	using System; 
	using System.Net.Sockets; 
	using System.Net; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Text;
	using System.Text.RegularExpressions;
	using System.Threading;
	using System.Runtime.Remoting.Messaging;

	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	
	/*
		包发送模块(与服务端网络部分的名称对应)
		处理网络数据的发送
	*/
    public abstract class PacketSenderBase 
    {
    	public delegate void AsyncSendMethod();

		protected NetworkInterfaceBase _networkInterface = null;
		AsyncCallback _asyncCallback = null;
		AsyncSendMethod _asyncSendMethod;
		
        public PacketSenderBase(NetworkInterfaceBase networkInterface)
        {
        	_networkInterface = networkInterface;
			_asyncSendMethod = new AsyncSendMethod(this._asyncSend);
			_asyncCallback = new AsyncCallback(_onSent);
        }

		~PacketSenderBase()
		{
		}

		public NetworkInterfaceBase networkInterface()
		{
			return _networkInterface;
		}

		public abstract bool send(MemoryStream stream);

		protected void _startSend()
		{
			// 由于socket用的是非阻塞式，因此在这里不能直接使用socket.send()方法
			// 必须放到另一个线程中去做
			_asyncSendMethod.BeginInvoke(_asyncCallback, null);
		}

		protected abstract void _asyncSend();
		
		protected static void _onSent(IAsyncResult ar)
		{
			AsyncResult result = (AsyncResult)ar;
			AsyncSendMethod caller = (AsyncSendMethod)result.AsyncDelegate;
			caller.EndInvoke(ar);
		}
	}
} 
