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

        public class ReceiveState
        {
            public AsyncReceiveMethod caller = null;
        }

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
			var v = new AsyncReceiveMethod(this._asyncReceive);

            ReceiveState state = new ReceiveState();
            state.caller = v;

            v.BeginInvoke(new AsyncCallback(_onRecv), state);
		}

		protected abstract void _asyncReceive();

		private void _onRecv(IAsyncResult ar)
		{
            ReceiveState state = (ReceiveState)ar.AsyncState; ;
            state.caller.EndInvoke(ar);
		}
	}
} 
