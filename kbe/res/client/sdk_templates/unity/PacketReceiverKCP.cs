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
	using MessageLengthEx = System.UInt32;
	
	/*
		包接收模块(与服务端网络部分的名称对应)
		处理网络数据的接收
	*/
	public class PacketReceiverKCP : PacketReceiverBase
	{
        private byte[] _buffer;
		private Deps.KCP kcp_ = null;

        public PacketReceiverKCP(NetworkInterfaceBase networkInterface) : base(networkInterface) 
		{
            _buffer = new byte[MessageLength.MaxValue + (Deps.KCP.IKCP_OVERHEAD * 2)];
            _messageReader = new MessageReaderKCP();

			kcp_ = ((NetworkInterfaceKCP)networkInterface).kcp();
		}

		~PacketReceiverKCP()
		{
			kcp_ = null;
			Dbg.DEBUG_MSG("PacketReceiverKCP::~PacketReceiverKCP(), destroyed!");
		}

		public override void process()
		{
			Socket socket = _networkInterface.sock();

			while (socket.Available > 0)
			{
				int length = 0;

				try
				{
                	length = socket.Receive(_buffer);
				}
				catch (Exception e)
				{
					Dbg.ERROR_MSG("PacketReceiverKCP::process: " + e.ToString());
					continue;
				}

                if (length <= 0)
                {
                    Dbg.WARNING_MSG("PacketReceiverKCP::_asyncReceive(): KCP Receive <= 0!");
                    return;
                }

				((NetworkInterfaceKCP)_networkInterface).nextTickKcpUpdate = 0; 
                if(kcp_.Input(_buffer, 0, length) < 0)
                {
                    Dbg.WARNING_MSG(string.Format("PacketReceiverKCP::_asyncReceive(): KCP Input get {0}!", length));
                    return;
                }

                while (true)
                {
                    length = kcp_.Recv(_buffer, 0, _buffer.Length);
                    if (length < 0)
                    {
                        break;
                    }

					_messageReader.process(_buffer, 0, (MessageLengthEx)length);
                }
			}
		}

		public override void startRecv()
		{
			//var v = new AsyncReceiveMethod(this._asyncReceive);
			//v.BeginInvoke(new AsyncCallback(_onRecv), null);
		}

		protected override void _asyncReceive()
		{
		}
	}
} 
