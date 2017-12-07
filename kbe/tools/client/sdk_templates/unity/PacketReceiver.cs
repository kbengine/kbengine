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
		包接收模块(与服务端网络部分的名称对应)
		处理网络数据的接收
	*/
	public class PacketReceiver
	{
		public delegate void AsyncReceiveMethod(); 

		private MessageReader messageReader = null;
		private NetworkInterface _networkInterface = null;

		private byte[] _buffer;

		// socket向缓冲区写的起始位置
		int _wpos = 0;

		// 主线程读取数据的起始位置
		int _rpos = 0;

		public PacketReceiver(NetworkInterface networkInterface)
		{
			_init(networkInterface);
		}

		~PacketReceiver()
		{
			Dbg.DEBUG_MSG("PacketReceiver::~PacketReceiver(), destroyed!");
		}

		void _init(NetworkInterface networkInterface)
		{
			_networkInterface = networkInterface;
			_buffer = new byte[KBEngineApp.app.getInitArgs().RECV_BUFFER_MAX];

			messageReader = new MessageReader();
		}

		public NetworkInterface networkInterface()
		{
			return _networkInterface;
		}

		public void process()
		{
			int t_wpos = Interlocked.Add(ref _wpos, 0);

			if (_rpos < t_wpos)
			{
				messageReader.process(_buffer, (UInt32)_rpos, (UInt32)(t_wpos - _rpos));
				Interlocked.Exchange(ref _rpos, t_wpos);
			}
			else if (t_wpos < _rpos)
			{
				messageReader.process(_buffer, (UInt32)_rpos, (UInt32)(_buffer.Length - _rpos));
				messageReader.process(_buffer, (UInt32)0, (UInt32)t_wpos);
				Interlocked.Exchange(ref _rpos, t_wpos);
			}
			else
			{
				// 没有可读数据
			}
		}

		int _free()
		{
			int t_rpos = Interlocked.Add(ref _rpos, 0);

			if (_wpos == _buffer.Length)
			{
				if (t_rpos == 0)
				{
					return 0;
				}

				Interlocked.Exchange(ref _wpos, 0);
			}

			if (t_rpos <= _wpos)
			{
				return _buffer.Length - _wpos;
			}

			return t_rpos - _wpos - 1;
		}

		public void startRecv()
		{

			var v = new AsyncReceiveMethod(this._asyncReceive);
			v.BeginInvoke(new AsyncCallback(_onRecv), null);
		}

		private void _asyncReceive()
		{
			if (_networkInterface == null || !_networkInterface.valid())
			{
				Dbg.WARNING_MSG("PacketReceiver::_asyncReceive(): network interface invalid!");
				return;
			}

			var socket = _networkInterface.sock();

			while (true)
			{
				// 必须有空间可写，否则我们阻塞在线程中直到有空间为止
				int first = 0;
				int space = _free();

				while (space == 0)
				{
					if (first > 0)
					{
						if (first > 1000)
						{
							Dbg.ERROR_MSG("PacketReceiver::_asyncReceive(): no space!");
							Event.fireIn("_closeNetwork", new object[] { _networkInterface });
							return;
						}

						Dbg.WARNING_MSG("PacketReceiver::_asyncReceive(): waiting for space, Please adjust 'RECV_BUFFER_MAX'! retries=" + first);
						System.Threading.Thread.Sleep(5);
					}

					first += 1;
					space = _free();
				}

				int bytesRead = 0;
				try
				{
					bytesRead = socket.Receive(_buffer, _wpos, space, 0);
				}
				catch (SocketException se)
				{
					Dbg.ERROR_MSG(string.Format("PacketReceiver::_asyncReceive(): receive error, disconnect from '{0}'! error = '{1}'", socket.RemoteEndPoint, se));
					Event.fireIn("_closeNetwork", new object[] { _networkInterface });
					return;
				}

				if (bytesRead > 0)
				{
					// 更新写位置
					Interlocked.Add(ref _wpos, bytesRead);
				}
				else
				{
					Dbg.WARNING_MSG(string.Format("PacketReceiver::_asyncReceive(): receive 0 bytes, disconnect from '{0}'!", socket.RemoteEndPoint));
					Event.fireIn("_closeNetwork", new object[] { _networkInterface });
					return;
				}
			}
		}

		private void _onRecv(IAsyncResult ar)
		{
			AsyncResult result = (AsyncResult)ar;
			AsyncReceiveMethod caller = (AsyncReceiveMethod)result.AsyncDelegate;
			caller.EndInvoke(ar);
		}
	}
} 
