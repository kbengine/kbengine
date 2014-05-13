namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Net.Sockets; 
	using System.Net; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Text;
	using System.Threading;
	
	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	
    public class NetworkInterface 
    {
    	public const int TCP_PACKET_MAX = 1460;
    	
        private Socket socket_ = null;
        private KBEngineApp app_ = null;
		private List<MemoryStream> packets_ = null;
		private MessageReader msgReader = new MessageReader();
		private static ManualResetEvent TimeoutObject = new ManualResetEvent(false);
		private static byte[] _datas = new byte[MemoryStream.BUFFER_MAX];
		
        public NetworkInterface(KBEngineApp app)
        {
        	this.app_ = app;
			bindMessage();
        	packets_ = new List<MemoryStream>();
        }
		
		public void reset()
		{
			if(valid())
				close();
			
			socket_ = null;
			msgReader = new MessageReader();
			packets_.Clear();
			TimeoutObject.Set();
		}
		
		public Socket sock()
		{
			return socket_;
		}
		
		public bool valid()
		{
			return ((socket_ != null) && (socket_.Connected == true));
		}
		
		public void bindMessage()
		{
			if(Message.messages.Count == 0)
			{
				Message.messages["Loginapp_importClientMessages"] = new Message(5, "importClientMessages", 0, 0, new List<Byte>(), null);
				Message.messages["Baseapp_importClientMessages"] = new Message(207, "importClientMessages", 0, 0, new List<Byte>(), null);
				Message.messages["Baseapp_importClientEntityDef"] = new Message(208, "importClientMessages", 0, 0, new List<Byte>(), null);
				
				Message.messages["Client_onImportClientMessages"] = new Message(518, "Client_onImportClientMessages", -1, -1, new List<Byte>(), 
					this.app_.GetType().GetMethod("Client_onImportClientMessages"));
				Message.clientMessages[Message.messages["Client_onImportClientMessages"].id] = Message.messages["Client_onImportClientMessages"];
			}
		}
		
		private static void connectCB(IAsyncResult asyncresult)
		{
			if(KBEngineApp.app.networkInterface().valid())
				KBEngineApp.app.networkInterface().sock().EndConnect(asyncresult);
			
			TimeoutObject.Set();
		}
	    
		public bool connect(string ip, int port) 
		{
			int count = 0;
__RETRY:
			reset();
			TimeoutObject.Reset();
			
			// Security.PrefetchSocketPolicy(ip, 843);
			socket_ = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp); 
			socket_.SetSocketOption (System.Net.Sockets.SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, MemoryStream.BUFFER_MAX);
			
            try 
            { 
                IPEndPoint endpoint = new IPEndPoint(IPAddress.Parse(ip), port); 
                
				socket_.BeginConnect(endpoint, new AsyncCallback(connectCB), socket_);
				
		        if (TimeoutObject.WaitOne(10000))
		        {
		        }
		        else
		        {
		        	reset();
		        }
        
            } 
            catch (Exception e) 
            {
                Dbg.WARNING_MSG(e.ToString());
                
                if(count < 3)
                {
                	Dbg.WARNING_MSG("connect(" + ip + ":" + port + ") is error, try=" + (count++) + "!");
                	goto __RETRY;
           		 }
            
				return false;
            } 
			
			return valid();
		}
        
        public void close()
        {
            socket_.Close(0);
            socket_ = null;
        }

        public void send(byte[] datas)
        {
           if(socket_ == null || socket_.Connected == false) 
			{
               throw new ArgumentException ("invalid socket!");
            }
			
            if (datas == null || datas.Length == 0 ) 
			{
                throw new ArgumentException ("invalid datas!");
            }
			
			try
			{
				socket_.Send(datas);
			}
			catch (SocketException err)
			{
                if (err.ErrorCode == 10054 || err.ErrorCode == 10053)
                {
					Dbg.DEBUG_MSG(string.Format("NetworkInterface::send(): disable connect!"));
					if(socket_ != null && socket_.Connected)
						socket_.Close();
					socket_ = null;
					Event.fire("onDisableConnect", new object[]{});
                }
				else{
					Dbg.ERROR_MSG(string.Format("NetworkInterface::send(): socket error(" + err.ErrorCode + ")!"));
				}
			}
        }
		
		public void recv()
		{
           if(socket_ == null || socket_.Connected == false) 
			{
				throw new ArgumentException ("invalid socket!");
            }
			
            if (socket_.Poll(1000, SelectMode.SelectRead))
            {
	           if(socket_ == null || socket_.Connected == false) 
				{
					Dbg.WARNING_MSG("invalid socket!");
					return;
	            }
				
				int successReceiveBytes = 0;
				
				try
				{
					successReceiveBytes = socket_.Receive(_datas, MemoryStream.BUFFER_MAX, 0);
				}
				catch (SocketException err)
				{
                    if (err.ErrorCode == 10054 || err.ErrorCode == 10053)
                    {
						Dbg.DEBUG_MSG(string.Format("NetworkInterface::recv(): disable connect!"));
						if(socket_ != null && socket_.Connected)
							socket_.Close();
						socket_ = null;
						Event.fire("onDisableConnect", new object[]{});
                    }
					else{
						Dbg.ERROR_MSG(string.Format("NetworkInterface::recv(): socket error(" + err.ErrorCode + ")!"));
					}
				}
				
				if(successReceiveBytes > 0)
				{
				//	Dbg.DEBUG_MSG(string.Format("NetworkInterface::recv(): size={0}!", successReceiveBytes));
				}
				else if(successReceiveBytes == 0)
				{
					Dbg.DEBUG_MSG(string.Format("NetworkInterface::recv(): disable connect!"));
					if(socket_ != null && socket_.Connected)
						socket_.Close();
					socket_ = null;
				}
				else
				{
					Dbg.ERROR_MSG(string.Format("NetworkInterface::recv(): socket error!"));
				}
				
				msgReader.process(_datas, (MessageLength)successReceiveBytes);
            }
		}
		
		public void process() 
		{
			if(socket_ != null && socket_.Connected)
			{
				recv();
			}
			else
			{
				System.Threading.Thread.Sleep(1);
			}
		}
	}
} 
