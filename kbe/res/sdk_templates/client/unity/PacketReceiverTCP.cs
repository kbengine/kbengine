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
	public class PacketReceiverTCP : PacketReceiverBase
	{
		private byte[] _buffer;

		// socket向缓冲区写的起始位置
		int _wpos = 0;

		// 主线程读取数据的起始位置
		int _rpos = 0;

		public PacketReceiverTCP(NetworkInterfaceBase networkInterface) : base(networkInterface) 
		{
			_buffer = new byte[KBEngineApp.app.getInitArgs().TCP_RECV_BUFFER_MAX];
			_messageReader = new MessageReaderTCP();
		}

		~PacketReceiverTCP()
		{
			Dbg.DEBUG_MSG("PacketReceiverTCP::~PacketReceiverTCP(), destroyed!");
		}

		public override void process()
		{
			int t_wpos = Interlocked.Add(ref _wpos, 0);

			if (_rpos < t_wpos)
			{
                if (_networkInterface.fileter() != null)
                {
                    _networkInterface.fileter().recv(_messageReader, _buffer, (UInt32)_rpos, (UInt32)(t_wpos - _rpos));
                }
                else
                {
                    _messageReader.process(_buffer, (UInt32)_rpos, (UInt32)(t_wpos - _rpos));
                }
                    
				Interlocked.Exchange(ref _rpos, t_wpos);
			}
			else if (t_wpos < _rpos)
			{
                if (_networkInterface.fileter() != null)
                {
                    _networkInterface.fileter().recv(_messageReader, _buffer, (UInt32)_rpos, (UInt32)(_buffer.Length - _rpos));
                    _networkInterface.fileter().recv(_messageReader, _buffer, (UInt32)0, (UInt32)t_wpos);
                }
                else
                {
                    _messageReader.process(_buffer, (UInt32)_rpos, (UInt32)(_buffer.Length - _rpos));
                    _messageReader.process(_buffer, (UInt32)0, (UInt32)t_wpos);
                }
                
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

		protected override void _asyncReceive()
		{
			if (_networkInterface == null || !_networkInterface.valid())
			{
				Dbg.WARNING_MSG("PacketReceiverTCP::_asyncReceive(): network interface invalid!");
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
							Dbg.ERROR_MSG("PacketReceiverTCP::_asyncReceive(): no space!");
							Event.fireIn("_closeNetwork", new object[] { _networkInterface });
							return;
						}

						Dbg.WARNING_MSG("PacketReceiverTCP::_asyncReceive(): waiting for space, Please adjust 'RECV_BUFFER_MAX'! retries=" + first);
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
					Dbg.ERROR_MSG(string.Format("PacketReceiverTCP::_asyncReceive(): receive error, disconnect from '{0}'! error = '{1}'", socket.RemoteEndPoint, se));
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
					Dbg.WARNING_MSG(string.Format("PacketReceiverTCP::_asyncReceive(): receive 0 bytes, disconnect from '{0}'!", socket.RemoteEndPoint));
					Event.fireIn("_closeNetwork", new object[] { _networkInterface });
					return;
				}
			}
		}
	}
} 
