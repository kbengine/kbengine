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

	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	
	/*
		包接收模块(与服务端网络部分的名称对应)
		处理网络数据的接收
	*/
	public abstract class PacketReceiverBase
	{
		public delegate void AsyncReceiveMethod(); 
		protected MessageReaderBase _messageReader = null;
		protected NetworkInterfaceBase _networkInterface = null;

		public PacketReceiverBase(NetworkInterfaceBase networkInterface)
		{
			_networkInterface = networkInterface;
		}

		~PacketReceiverBase()
		{
		}

		public NetworkInterfaceBase networkInterface()
		{
			return _networkInterface;
		}

		public abstract void process();

		public virtual void startRecv()
		{
			AsyncReceiveMethod asyncReceiveMethod = new AsyncReceiveMethod(this._asyncReceive);
			asyncReceiveMethod.BeginInvoke(new AsyncCallback(_onRecv), asyncReceiveMethod);
		}

		protected abstract void _asyncReceive();

		private void _onRecv(IAsyncResult ar)
		{
			try
			{
				AsyncReceiveMethod caller = (AsyncReceiveMethod)ar.AsyncState;
				caller.EndInvoke(ar);
			}
			catch(ObjectDisposedException)
			{
				//通常出现这个错误, 是因为longin_baseapp时, networkInterface已经reset, _packetReceiver被置为null, 而之后刚好该回调被调用
			}
		}
	}
} 
