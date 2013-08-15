namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Text;
    using System.Threading;
	
	using MessageID = System.UInt16;
	using MessageLength = System.UInt16;
	
    class KBEThread
    {

        KBEngineApp app_;

        public KBEThread(KBEngineApp app)
        {
            this.app_ = app;
        }

        public void run()
        {
			MonoBehaviour.print("KBEThread::run()");
			
            try
            {
                this.app_.process();
            }
            catch (Exception e)
            {
                MonoBehaviour.print(e.ToString());
                this.app_.t().Abort();
            }
			
			MonoBehaviour.print("KBEThread::end()");
        }

    }

	public class KBEngineApp
	{
		public static KBEngineApp app = null;
		private NetworkInterface networkInterface_ = null;
		
        private Thread t_ = null;
        private KBEThread kbethread_ = null;
        
        public string username = "kbengine";
        public string password = "123456";
        
		private bool loginappMessageImported_ = false;
		private bool baseappMessageImported_ = false;
		private bool entitydefImported_ = false;
		
		public string ip = "127.0.0.1";
		public UInt16 port = 20013;
		
		private string currserver_ = "loginapp";
		private string currstate_ = "create";
		private byte[] serverdatas_ = new byte[0];
		private string clientdatas_ = "";
		private string serverVersion_ = "";
		private string clientVersion_ = "0.0.1";
		
		public UInt64 entity_uuid = 0;
		public Int32 entity_id = 0;
		public string entity_type = "";
		
		public Dictionary<Int32, Entity> entities = new Dictionary<Int32, Entity>();
		
		private UInt64 lastticktime_ = 0;
		public UInt32 spaceID = 0;
		public string spaceResPath = "";
		
		public EntityDef entityDef = new EntityDef();
		
		public bool isbreak = false;
		
        public KBEngineApp()
        {
			app = this;

        	networkInterface_ = new NetworkInterface(this);
            kbethread_ = new KBEThread(this);
            t_ = new Thread(new ThreadStart(kbethread_.run));
            t_.Start();
        }
		
        public Thread t(){
        	return t_;
        }
        
        public NetworkInterface networkInterface(){
        	return networkInterface_;
        }
        
		public void reset()
		{
			loginappMessageImported_ = false;
			baseappMessageImported_ = false;
			entitydefImported_ = false;
			
			ip = "127.0.0.1";
			port = 20013;
			
			currserver_ = "loginapp";
			currstate_ = "create";
			serverdatas_ = new byte[0];
			clientdatas_ = "";
			serverVersion_ = "";
			clientVersion_ = "0.0.1";
			
			entity_uuid = 0;
			entity_id = 0;
			entity_type = "";
			
			entities.Clear();
			
			lastticktime_ = 0;
			spaceID = 0;
			spaceResPath = "";
		}
		
		public void process()
		{
			while(!isbreak)
			{
				networkInterface_.process();
			}
		}
		
		public Entity player(){
			Entity e;
			if(entities.TryGetValue(entity_id, out e))
				return e;
			
			return null;
		}
		
		public void hello()
		{
		}
		
		public bool login_loginapp(bool noconnect)
		{
			if(noconnect)
			{
				if(!networkInterface_.connect(ip, port))
				{
					Debug.LogError(string.Format("KBEngine::login_loginapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onLogin_loginapp();
				Debug.Log(string.Format("KBEngine::login_loginapp(): connect {0}:{1} is successfylly!", ip, port));
			}
			else
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_login"]);
				bundle.writeInt8(3); // clientType
				bundle.writeBlob(new byte[0]);
				bundle.writeString(username);
				bundle.writeString(password);
				bundle.send(networkInterface_);
			}
			
			return true;
		}
		
		private void onLogin_loginapp()
		{
			currserver_ = "loginapp";
			currstate_ = "login";
			
			if(!loginappMessageImported_)
			{
				var bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Debug.Log("KBEngine::onLogin_loginapp: start importClientMessages ...");
			}
			else
			{
				onImportClientMessagesCompleted();
			}
		}
		
		public bool login_baseapp(bool noconnect)
		{  
			if(noconnect)
			{
				if(!networkInterface_.connect(ip, port))
				{
					Debug.LogError(string.Format("KBEngine::login_baseapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onLogin_baseapp();
				Debug.Log(string.Format("KBEngine::login_baseapp(): connect {0}:{1} is successfylly!", ip, port));
			}
			else
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Baseapp_loginGateway"]);
				bundle.writeString(username);
				bundle.writeString(password);
				bundle.send(networkInterface_);
			}
			
			return true;
		}
	
		private void onLogin_baseapp()
		{
			currserver_ = "baseapp";
			
			if(!baseappMessageImported_)
			{
				var bundle = new Bundle();
				bundle.newMessage(Message.messages["Baseapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Debug.Log("KBEngine::onLogin_baseapp: start importClientMessages ...");
			}
			else
			{
				onImportClientMessagesCompleted();
			}
		}
		
		private void onImportClientMessagesCompleted()
		{
			Debug.Log("KBEngine::onImportClientMessagesCompleted: successfully!");

			hello();
			
			if(currserver_ == "loginapp")
			{
				if(currstate_ == "login")
					login_loginapp(false);
				else if(currstate_ == "resetpassword")
					resetpassword_loginapp(false);
				else
					createAccount_loginapp(false);
				
				loginappMessageImported_ = true;
			}
			else
			{
				baseappMessageImported_ = true;
				
				if(!entitydefImported_)
				{
					Debug.Log("KBEngine::onImportClientMessagesCompleted: start importEntityDef ...");
					Bundle bundle = new Bundle();
					bundle.newMessage(Message.messages["Baseapp_importClientEntityDef"]);
					bundle.send(networkInterface_);
				}
				else
				{
					onImportEntityDefCompleted();
				}
			}
		}
		
		private void onImportEntityDefCompleted()
		{
			Debug.Log("KBEngine::onImportEntityDefCompleted: successfully!");
			entitydefImported_ = true;
			login_baseapp(false);
		}

		public void Client_onImportClientMessages(MemoryStream stream)
		{
			UInt16 msgcount = stream.readUint16();
			
			Debug.Log(string.Format("KBEngine::Client_onImportClientMessages: start({0})...", msgcount));
			
			while(msgcount > 0)
			{
				msgcount--;
				
				MessageID msgid = stream.readUint16();
				Int16 msglen = stream.readInt16();
				
				string msgname = stream.readString();
				Byte argsize = stream.readUint8();
				List<Byte> argstypes = new List<Byte>();
				
				for(Byte i=0; i<argsize; i++)
				{
					argstypes.Add(stream.readUint8());
				}
				
				System.Reflection.MethodInfo handler = null;
				bool isClientMethod = msgname.Contains("Client_");
				
				if(isClientMethod)
				{
					handler = GetType().GetMethod(msgname);
					if(handler == null)
					{
						Debug.LogWarning(string.Format("KBEngine::onImportClientMessages[{0}]: interface({1}/{2}/{3}) no implement!", 
							currserver_, msgname, msgid, msglen));
						handler = null;
					}
					else
					{
						Debug.Log(string.Format("KBEngine::onImportClientMessages: imported({0}/{1}/{2}) successfully!", 
							msgname, msgid, msglen));
					}
				}
				
				if(msgname.Length > 0)
				{
					Message.messages[msgname] = new Message(msgid, msgname, msglen, argstypes, handler);
					
					if(isClientMethod)
					{
						Message.clientMessages[msgid] = Message.messages[msgname];
					}
					else
					{
						if(currserver_ == "loginapp")
							Message.loginappMessages[msgid] = Message.messages[msgname];
						else
							Message.baseappMessages[msgid] = Message.messages[msgname];
					}
				}
				else
				{
					Message msg = new Message(msgid, msgname, msglen, argstypes, handler);
					if(currserver_ == "loginapp")
						Message.loginappMessages[msgid] = msg;
					else
						Message.baseappMessages[msgid] = msg;
				}
			};

			onImportClientMessagesCompleted();
		}
		
		public void resetpassword_loginapp(bool noconnect)
		{
		}
		
		public void createAccount_loginapp(bool noconnect)
		{
		}
		
		public void Client_onHelloCB(MemoryStream stream)
		{
			serverVersion_ = stream.readString();
			Int32 ctype = stream.readInt32();
			Debug.Log("KBEngine::Client_onHelloCB: verInfo(" + serverVersion_ + "), ctype(" + ctype + ")!");
		}
		
		public void Client_onLoginFailed(MemoryStream stream)
		{
			UInt16 failedcode = stream.readUint16();
			serverdatas_ = stream.readBlob();
			Debug.Log("KBEngine::Client_onLoginFailed: failedcode(" + failedcode + "), datas(" + serverdatas_.Length + ")!");
		}
		
		public void Client_onLoginSuccessfully(MemoryStream stream)
		{
			var accountName = stream.readString();
			username = accountName;
			ip = stream.readString();
			port = stream.readUint16();
			
			Debug.Log("KBEngine::Client_onLoginSuccessfully: accountName(" + accountName + "), addr(" + 
					ip + ":" + port + "), datas(" + serverdatas_.Length + ")!");
			
			serverdatas_ = stream.readBlob();
			login_baseapp(true);
		}
		
		public void Client_onLoginGatewayFailed(UInt16 failedcode)
		{
			Debug.Log("KBEngine::Client_onLoginGatewayFailed: failedcode(" + failedcode + ")!");
		}
		
	}
} 
