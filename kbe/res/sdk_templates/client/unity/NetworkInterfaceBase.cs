namespace KBEngine
{
	using UnityEngine;
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

	/// <summary>
	/// 网络模块
	/// 处理连接、收发数据
	/// </summary>
	public abstract class NetworkInterfaceBase
	{
		public const int TCP_PACKET_MAX = 1460;
		public const int UDP_PACKET_MAX = 1472;
		public const string UDP_HELLO = "62a559f3fa7748bc22f8e0766019d498";
		public const string UDP_HELLO_ACK = "1432ad7c829170a76dd31982c3501eca";

		public delegate void AsyncConnectMethod(ConnectState state);
		public delegate void ConnectCallback(string ip, int port, bool success, object userData);

		protected Socket _socket = null;
		protected PacketReceiverBase _packetReceiver = null;
		protected PacketSenderBase _packetSender = null;
		protected EncryptionFilter _filter = null;

		public bool connected = false;
		
		public class ConnectState
		{
			// for connect
			public string connectIP = "";
			public int connectPort = 0;
			public ConnectCallback connectCB = null;
			public AsyncConnectMethod caller = null;
			public object userData = null;
			public Socket socket = null;
			public NetworkInterfaceBase networkInterface = null;
			public string error = "";
		}
		
		public NetworkInterfaceBase()
		{
			reset();
		}

		~NetworkInterfaceBase()
		{
			Dbg.DEBUG_MSG("NetworkInterfaceBase::~NetworkInterfaceBase(), destructed!!!");
			reset();
		}

		public virtual Socket sock()
		{
			return _socket;
		}
		
		public virtual void reset()
		{
			_packetReceiver = null;
			_packetSender = null;
			_filter = null;
			connected = false;

			if(_socket != null)
			{
				try
				{
					if(_socket.RemoteEndPoint != null)
						Dbg.DEBUG_MSG(string.Format("NetworkInterfaceBase::reset(), close socket from '{0}'", _socket.RemoteEndPoint.ToString()));
				}
				catch (Exception e)
				{

				}

				_socket.Close(0);
				_socket = null;
			}
		}
		

		public virtual void close()
		{
			if(_socket != null)
			{
				_socket.Close(0);
				_socket = null;
				Event.fireAll(EventOutTypes.onDisconnected);
			}

			_socket = null;
			connected = false;
		}

		protected abstract PacketReceiverBase createPacketReceiver();
		protected abstract PacketSenderBase createPacketSender();
		protected abstract Socket createSocket();
		protected abstract void onAsyncConnect(ConnectState state);

		public virtual PacketReceiverBase packetReceiver()
		{
			return _packetReceiver;
		}

		public virtual PacketSenderBase PacketSender()
		{
			return _packetSender;
		}

		public virtual bool valid()
		{
			return ((_socket != null) && (_socket.Connected == true));
		}
		
		public void _onConnectionState(ConnectState state)
		{
			KBEngine.Event.deregisterIn(this);

			bool success = (state.error == "" && valid());
			if (success)
			{
				Dbg.DEBUG_MSG(string.Format("NetworkInterfaceBase::_onConnectionState(), connect to {0}:{1} is success!", state.connectIP, state.connectPort));
				_packetReceiver = createPacketReceiver();
				_packetReceiver.startRecv();
				connected = true;
			}
			else
			{
				reset();
				Dbg.ERROR_MSG(string.Format("NetworkInterfaceBase::_onConnectionState(), connect error! ip: {0}:{1}, err: {2}", state.connectIP, state.connectPort, state.error));
			}

			Event.fireAll(EventOutTypes.onConnectionState, success);

			if (state.connectCB != null)
				state.connectCB(state.connectIP, state.connectPort, success, state.userData);
		}

		private static void connectCB(IAsyncResult ar)
		{
			ConnectState state = null;
			
			try 
			{
				// Retrieve the socket from the state object.
				state = (ConnectState) ar.AsyncState;

				// Complete the connection.
				state.socket.EndConnect(ar);

				Event.fireIn("_onConnectionState", new object[] { state });
			} 
			catch (Exception e) 
			{
				state.error = e.ToString();
				Event.fireIn("_onConnectionState", new object[] { state });
			}
		}

		/// <summary>
		/// 在非主线程执行：连接服务器
		/// </summary>
		private void _asyncConnect(ConnectState state)
		{
			Dbg.DEBUG_MSG(string.Format("NetworkInterfaceBase::_asyncConnect(), will connect to '{0}:{1}' ...", state.connectIP, state.connectPort));
			onAsyncConnect(state);
		}

		protected virtual void onAsyncConnectCB(ConnectState state)
		{

		}

		/// <summary>
		/// 在非主线程执行：连接服务器结果回调
		/// </summary>
		private void _asyncConnectCB(IAsyncResult ar)
		{
			ConnectState state = (ConnectState)ar.AsyncState;
			
			onAsyncConnectCB(state);

			Dbg.DEBUG_MSG(string.Format("NetworkInterfaceBase::_asyncConnectCB(), connect to '{0}:{1}' finish. error = '{2}'", state.connectIP, state.connectPort, state.error));

			// Call EndInvoke to retrieve the results.
			state.caller.EndInvoke(ar);
			Event.fireIn("_onConnectionState", new object[] { state });
		}

		public void connectTo(string ip, int port, ConnectCallback callback, object userData)
		{
			if (valid())
				throw new InvalidOperationException("Have already connected!");

			if (!(new Regex(@"((?:(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d)))\.){3}(?:25[0-5]|2[0-4]\d|((1\d{2})|([1-9]?\d))))")).IsMatch(ip))
			{
				IPHostEntry ipHost = Dns.GetHostEntry(ip);
				ip = ipHost.AddressList[0].ToString();
			}

			_socket = createSocket();

			AsyncConnectMethod asyncConnectMethod = new AsyncConnectMethod(this._asyncConnect);

			ConnectState state = new ConnectState();
			state.connectIP = ip;
			state.connectPort = port;
			state.connectCB = callback;
			state.userData = userData;
			state.socket = _socket;
			state.networkInterface = this;
			state.caller = asyncConnectMethod;

			Dbg.DEBUG_MSG("connect to " + ip + ":" + port + " ...");
			connected = false;
			
			// 先注册一个事件回调，该事件在当前线程触发
			Event.registerIn("_onConnectionState", this, "_onConnectionState");

			asyncConnectMethod.BeginInvoke(state, new AsyncCallback(this._asyncConnectCB), state);
		}

		public virtual bool send(MemoryStream stream)
		{
			if (!valid())
			{
				throw new ArgumentException("invalid socket!");
			}

			if (_packetSender == null)
				_packetSender = createPacketSender();

			if (_filter != null)
				return _filter.send(_packetSender, stream);

			return _packetSender.send(stream);
		}

		public virtual void process()
		{
			if (!valid())
				return;

			if (_packetReceiver != null)
				_packetReceiver.process();
		}


		public EncryptionFilter fileter()
		{
			return _filter;
		}

		public void setFilter(EncryptionFilter filter)
		{
			_filter = filter;
		}
	}
}
