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
	public class NetworkInterfaceKCP : NetworkInterfaceBase
	{
		private Deps.KCP kcp_ = null;
		public UInt32 connID;
		public UInt32 nextTickKcpUpdate = 0;
		public EndPoint remoteEndPint = null;

		protected override Socket createSocket()
		{
			Socket pSocket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
			return pSocket;
		}

		protected override PacketReceiverBase createPacketReceiver()
		{
			return new PacketReceiverKCP(this);
		}

		protected override PacketSenderBase createPacketSender()
		{
			return new PacketSenderKCP(this);
		}

		public override void reset()
		{
			finiKCP();
			base.reset();
		}
		
        public override void close()
        {
			finiKCP();
			base.close();
        }

		public override bool valid()
		{
			return ((kcp_ != null) && (_socket != null) && connected);
		}

		protected void outputKCP(byte[] data, int size, object userData)
		{
			if (!valid())
			{
				throw new ArgumentException("invalid socket!");
			}

			if (_packetSender == null)
				_packetSender = createPacketSender();

			((PacketSenderKCP)_packetSender).sendto(data, size);
		}

		bool initKCP()
		{
            kcp_ = new Deps.KCP(connID, this);
            kcp_.SetOutput(outputKCP);

            kcp_.SetMTU(1400);
            kcp_.WndSize(KBEngineApp.app.getInitArgs().getUDPSendBufferSize(), KBEngineApp.app.getInitArgs().getUDPRecvBufferSize());
            kcp_.NoDelay(1, 10, 2, 1);
            kcp_.SetMinRTO(10);

			nextTickKcpUpdate = 0;
			return true;
		}

		bool finiKCP()
		{
			if(kcp_ != null)
			{
				kcp_.SetOutput(null);
				kcp_.Release();
				kcp_ = null;
			}

			remoteEndPint = null;
			connID = 0;
			nextTickKcpUpdate = 0;
			return true;
		}

		public Deps.KCP kcp()
		{
			return kcp_;
		}

		public override bool send(MemoryStream stream)
		{
			if (!valid())
			{
				throw new ArgumentException("invalid socket!");
			}

            if(_filter != null)
            {
                _filter.encrypt(stream);
            }

			nextTickKcpUpdate = 0;
			return kcp_.Send(stream.data(), stream.rpos, (int)stream.length()) >= 0;
		}

		public override void process()
		{
			if (!valid())
				return;

			uint current = Deps.KCP.TimeUtils.iclock();
			if(current >= nextTickKcpUpdate)
			{
				kcp_.Update(current);
				nextTickKcpUpdate = kcp_.Check(current);
			}

			if (_packetReceiver != null)
				_packetReceiver.process();
		}

		protected override void onAsyncConnectCB(ConnectState state)
		{
			if(state.error.Length > 0 || !initKCP())
				return;

			connected = true;
			remoteEndPint = new IPEndPoint(IPAddress.Parse(state.connectIP), state.connectPort);
		}

		protected override void onAsyncConnect(ConnectState state)
		{
			try
			{
				//state.socket.Connect(state.connectIP, state.connectPort);

				byte[] helloPacket = System.Text.Encoding.ASCII.GetBytes(UDP_HELLO);
				state.socket.SendTo(helloPacket, helloPacket.Length, SocketFlags.None, new IPEndPoint(IPAddress.Parse(state.connectIP), state.connectPort));

                ArrayList readList = new ArrayList();
                readList.Add(state.socket);
                Socket.Select(readList, null, null, 3000000);

				if(readList.Count > 0)
				{
					byte[] buffer = new byte[UDP_PACKET_MAX];
					int length = state.socket.Receive(buffer);

                    if (length <= 0)
                    {
						Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), failed to connect to '{0}:{1}'! receive hello-ack error!", state.connectIP, state.connectPort));
						state.error = "receive hello-ack error!";
					}
                    else
                    {
                        MemoryStream stream = new MemoryStream();
                        Array.Copy(buffer, 0, stream.data(), stream.wpos, length);
                        stream.wpos = length;
                        string helloAck = stream.readString();
                        string versionString = stream.readString();
                        uint conv = stream.readUint32();

                        if (helloAck != UDP_HELLO_ACK)
                        {
                            Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), failed to connect to '{0}:{1}'! receive hello-ack({2}!={3}) mismatch!",
                                state.connectIP, state.connectPort, helloAck, UDP_HELLO_ACK));

                            state.error = "hello-ack mismatch!";
                        }
						else if(KBEngineApp.app.serverVersion != versionString)
						{
                            Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), failed to connect to '{0}:{1}'! version({2}!={3}) mismatch!",
                                state.connectIP, state.connectPort, versionString, KBEngineApp.app.serverVersion));

                            state.error = "version mismatch!";
						}
						else if(conv == 0)
						{
                            Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), failed to connect to '{0}:{1}'! conv is 0!",
                                state.connectIP, state.connectPort));

                            state.error = "kcp conv error!";
						}

						((NetworkInterfaceKCP)state.networkInterface).connID = conv;
                    }
				}
				else
				{
					Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), connect to '{0}:{1}' timeout!'", state.connectIP, state.connectPort));
					state.error = "timeout!";
				}
			}
			catch (Exception e)
			{
				Dbg.ERROR_MSG(string.Format("NetworkInterfaceKCP::_asyncConnect(), connect to '{0}:{1}' fault! error = '{2}'", state.connectIP, state.connectPort, e));
				state.error = e.ToString();
			}
		}
	}
}
