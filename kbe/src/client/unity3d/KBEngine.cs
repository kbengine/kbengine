namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Text;
    using System.Threading;
	using System.Text.RegularExpressions;
	
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
			int count = 0;
START_RUN:
            try
            {
                this.app_.process();
                count = 0;
            }
            catch (Exception e)
            {
                MonoBehaviour.print(e.ToString());
                MonoBehaviour.print("KBEThread::try run:" + count);
                count ++;
                if(count < 10)
                	goto START_RUN;
            }
			
			MonoBehaviour.print("KBEThread::end()");
        }
    }

	public class KBEngineApp
	{
		public static KBEngineApp app = null;
		public float KBE_FLT_MAX = float.MaxValue;
		private NetworkInterface networkInterface_ = null;
		
        private Thread t_ = null;
        private KBEThread kbethread_ = null;
        
        public string username = "kbengine";
        public string password = "123456";
        
		private static bool loginappMessageImported_ = false;
		private static bool baseappMessageImported_ = false;
		private static bool entitydefImported_ = false;
		private static bool isImportMercuryErrorsDescr_ = false;
		
		public static string url = "http://127.0.0.1";
		public string ip = "127.0.0.1";
		public UInt16 port = 20013;
		
		public string loginappIP = "";
		public UInt16 loginappPort = 0;
		
		/*
		public static string url = "http://117.135.154.72:20080";
		public string ip = "117.135.154.72";
		public UInt16 port = 20034;
		*/
		
		private string currserver_ = "loginapp";
		private string currstate_ = "create";
		private byte[] serverdatas_ = new byte[0];
		private byte[] clientdatas_ = new byte[0];
		private string serverVersion_ = "";
		private string clientVersion_ = "0.1.4";
		
		public UInt64 entity_uuid = 0;
		public Int32 entity_id = 0;
		public string entity_type = "";
		public Vector3 entityLastLocalPos = new Vector3(0f, 0f, 0f);
		public Vector3 entityLastLocalDir = new Vector3(0f, 0f, 0f);
		public Vector3 entityServerPos = new Vector3(0f, 0f, 0f);
		Dictionary<string, string> spacedatas = new Dictionary<string, string>();
		
		public Dictionary<Int32, Entity> entities = new Dictionary<Int32, Entity>();
		public List<Int32> entityIDAliasIDList = new List<Int32>();
		private Dictionary<Int32, MemoryStream> bufferedCreateEntityMessage = new Dictionary<Int32, MemoryStream>(); 
		
		public static SkillBox skillbox = new SkillBox();
		
		public struct MercuryErr
		{
			public string name;
			public string descr;
			public UInt16 id;
		}
		
		public static Dictionary<UInt16, MercuryErr> mercuryErrs = new Dictionary<UInt16, MercuryErr>(); 
		
		private System.DateTime lastticktime_ = System.DateTime.Now;
		private System.DateTime lastUpdateToServerTime_ = System.DateTime.Now;
		
		public UInt32 spaceID = 0;
		public string spaceResPath = "";
		
		public static EntityDef entityDef = new EntityDef();
		
		public bool isbreak = false;
		
        public KBEngineApp()
        {
			app = this;

			loginappIP = ip;
			loginappPort = port;
		
        	networkInterface_ = new NetworkInterface(this);
            kbethread_ = new KBEThread(this);
            t_ = new Thread(new ThreadStart(kbethread_.run));
            t_.Start();
        }

        public void destroy(){
        	isbreak = true;
        	t_.Abort();
        	t_ = null;
        	
        	reset();
        }
        
        public Thread t(){
        	return t_;
        }
        
        public NetworkInterface networkInterface(){
        	return networkInterface_;
        }
        
		public void reset()
		{
			ip = loginappIP;
			port = loginappPort;
			
			currserver_ = "loginapp";
			currstate_ = "create";
			serverdatas_ = new byte[0];
			clientdatas_ = new byte[0];
			serverVersion_ = "";
			
			entity_uuid = 0;
			entity_id = 0;
			entity_type = "";
			
			entities.Clear();
			entityIDAliasIDList.Clear();
			bufferedCreateEntityMessage.Clear();
			
			lastticktime_ = System.DateTime.Now;
			lastUpdateToServerTime_ = System.DateTime.Now;
			spaceID = 0;
			spaceResPath = "";
			
			networkInterface_.reset();
			
			skillbox.clear();
			spacedatas.Clear();
		}
		
		public static bool validEmail(string strEmail) 
		{ 
			return Regex.IsMatch(strEmail, @"^([\w-\.]+)@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.)|(([\w-]+\.)+))([a-zA-Z]{2,4}|[0-9]{1,3})(\]?)$"); 
		}  
				
		public static Type GetTypeByString(string type)
	    {
	        switch (type.ToLower())
	        {
	            case "bool":
	                return Type.GetType("System.Boolean", true, true);
	            case "byte":
	                return Type.GetType("System.Byte", true, true);
	            case "sbyte":
	                return Type.GetType("System.SByte", true, true);
	            case "char":
	                return Type.GetType("System.Char", true, true);
	            case "decimal":
	                return Type.GetType("System.Decimal", true, true);
	            case "double":
	                return Type.GetType("System.Double", true, true);
	            case "float":
	                return Type.GetType("System.Single", true, true);
	            case "int":
	                return Type.GetType("System.Int32", true, true);
	            case "uint":
	                return Type.GetType("System.UInt32", true, true);
	            case "long":
	                return Type.GetType("System.Int64", true, true);
	            case "ulong":
	                return Type.GetType("System.UInt64", true, true);
	            case "object":
	                return Type.GetType("System.Object", true, true);
	            case "short":
	                return Type.GetType("System.Int16", true, true);
	            case "ushort":
	                return Type.GetType("System.UInt16", true, true);
	            case "string":
	                return Type.GetType("System.String", true, true);
	            case "date":
	            case "datetime":
	                return Type.GetType("System.DateTime", true, true);
	            case "guid":
	                return Type.GetType("System.Guid", true, true);
	            default:
	                return Type.GetType(type, true, true);
	        }
	    }
		
		public void process()
		{
			while(!isbreak)
			{
				networkInterface_.process();
				sendTick();
			}
			
			Dbg.WARNING_MSG("KBEngine::process(): break!");
		}
		
		public Entity player(){
			Entity e;
			if(entities.TryGetValue(entity_id, out e))
				return e;
			
			return null;
		}

		public void sendTick()
		{
			if(!networkInterface_.valid())
				return;
			
			TimeSpan span = DateTime.Now - lastticktime_; 
			
			updatePlayerToServer();
			
			if(span.Seconds > 15)
			{
				if(Message.messages["Loginapp_onClientActiveTick"] != null || Message.messages["Baseapp_onClientActiveTick"] != null)
				{
					Bundle bundle = new Bundle();
					if(currserver_ == "loginapp")
					{
						bundle.newMessage(Message.messages["Loginapp_onClientActiveTick"]);
					}
					else
					{
						bundle.newMessage(Message.messages["Baseapp_onClientActiveTick"]);
					}

					bundle.send(networkInterface_);
				}
				
				lastticktime_ = System.DateTime.Now;
			}
		}
		
		public void hello()
		{
			Bundle bundle = new Bundle();
			if(currserver_ == "loginapp")
				bundle.newMessage(Message.messages["Loginapp_hello"]);
			else
				bundle.newMessage(Message.messages["Baseapp_hello"]);
			bundle.writeString(clientVersion_);
			bundle.writeBlob(clientdatas_);
			bundle.send(networkInterface_);
		}

		public void Client_onVersionNotMatch(MemoryStream stream)
		{
			serverVersion_ = stream.readString();
			Dbg.DEBUG_MSG("Client_onVersionNotMatch: verInfo=" + clientVersion_ + " not match(server: " + serverVersion_ + ")");
			
			Event.fire("onVersionNotMatch", new object[]{clientVersion_, serverVersion_});
		}
		
		public void Client_onImportMercuryErrorsDescr(MemoryStream stream)
		{
			UInt16 size = stream.readUint16();
			while(size > 0)
			{
				size -= 1;
				
				MercuryErr e;
				e.id = stream.readUint16();
				e.name = stream.readString();
				e.descr = stream.readString();
				
				mercuryErrs.Add(e.id, e);
					
				Dbg.DEBUG_MSG("Client_onImportMercuryErrorsDescr: id=" + e.id + ", name=" + e.name + ", descr=" + e.descr);
			}
		}
		
		public bool login_loginapp(bool noconnect)
		{
			if(noconnect)
			{
				reset();
				if(!networkInterface_.connect(loginappIP, loginappPort))
				{
					Dbg.ERROR_MSG(string.Format("KBEngine::login_loginapp(): connect {0}:{1} is error!", loginappIP, loginappPort));  
					return false;
				}
				
				onLogin_loginapp();
				Dbg.DEBUG_MSG(string.Format("KBEngine::login_loginapp(): connect {0}:{1} is successfylly!", loginappIP, loginappPort));
			}
			else
			{
				Dbg.DEBUG_MSG("KBEngine::login_loginapp(): send login! username=" + username);
				
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
				Dbg.DEBUG_MSG("KBEngine::onLogin_loginapp: start importClientMessages ...");
				Event.fire("Loginapp_importClientMessages", new object[]{});
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
				Event.fire("login_baseapp", new object[]{});
				if(!networkInterface_.connect(ip, port))
				{
					Dbg.ERROR_MSG(string.Format("KBEngine::login_baseapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onLogin_baseapp();
				Dbg.DEBUG_MSG(string.Format("KBEngine::login_baseapp(): connect {0}:{1} is successfylly!", ip, port));
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
			currstate_ = "";
			
			if(!baseappMessageImported_)
			{
				var bundle = new Bundle();
				bundle.newMessage(Message.messages["Baseapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Dbg.DEBUG_MSG("KBEngine::onLogin_baseapp: start importClientMessages ...");
				Event.fire("Baseapp_importClientMessages", new object[]{});
			}
			else
			{
				onImportClientMessagesCompleted();
			}
		}
		
		public bool autoImportMessagesFromServer(bool isLoginapp)
		{  
			reset();
			if(!networkInterface_.connect(ip, port))
			{
				Dbg.ERROR_MSG(string.Format("KBEngine::autoImportMessagesFromServer(): connect {0}:{1} is error!", ip, port));
				return false;
			}

			if(isLoginapp)
			{
				currserver_ = "loginapp";
				currstate_ = "autoimport";
				
				if(!loginappMessageImported_)
				{
					var bundle = new Bundle();
					bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
					bundle.send(networkInterface_);
					Dbg.DEBUG_MSG("KBEngine::autoImportMessagesFromServer: start importClientMessages ...");
				}
				else
				{
					onImportClientMessagesCompleted();
				}
			}
			else{
				currserver_ = "baseapp";
				currstate_ = "autoimport";
				
				if(!baseappMessageImported_)
				{
					var bundle = new Bundle();
					bundle.newMessage(Message.messages["Baseapp_importClientMessages"]);
					bundle.send(networkInterface_);
					Dbg.DEBUG_MSG("KBEngine::autoImportMessagesFromServer: start importClientMessages ...");
				}
				else
				{
					onImportClientMessagesCompleted();
				}
			}
			
			Dbg.DEBUG_MSG(string.Format("KBEngine::autoImportMessagesFromServer(): connect {0}:{1} is successfylly!", ip, port));
			return true;
		}
	

		private void onImportClientMessagesCompleted()
		{
			Dbg.DEBUG_MSG("KBEngine::onImportClientMessagesCompleted: successfully! currserver=" + 
				currserver_ + ", currstate=" + currstate_);

			hello();
			
			if(currserver_ == "loginapp")
			{
				if(isImportMercuryErrorsDescr_ == false)
				{
					Dbg.DEBUG_MSG("KBEngine::onImportClientMessagesCompleted(): start importMercuryErrorsDescr!");
					isImportMercuryErrorsDescr_ = true;
					Bundle bundle = new Bundle();
					bundle.newMessage(Message.messages["Loginapp_importMercuryErrorsDescr"]);
					bundle.send(networkInterface_);
				}
				
				if(currstate_ == "login")
				{
					login_loginapp(false);
				}
				else if(currstate_ == "autoimport")
				{
				}
				else if(currstate_ == "resetpassword")
				{
					resetpassword_loginapp(false);
				}
				else
				{
					createAccount_loginapp(false);
				}
				
				loginappMessageImported_ = true;
			}
			else
			{
				baseappMessageImported_ = true;
				
				if(entitydefImported_ == false)
				{
					Dbg.DEBUG_MSG("KBEngine::onImportClientMessagesCompleted: start importEntityDef ...");
					Bundle bundle = new Bundle();
					bundle.newMessage(Message.messages["Baseapp_importClientEntityDef"]);
					bundle.send(networkInterface_);
					Event.fire("Baseapp_importClientEntityDef", new object[]{});
				}
				else
				{
					onImportEntityDefCompleted();
				}
			}
		}
		
		public void createDataTypeFromStream(MemoryStream stream, bool canprint)
		{
			UInt16 utype = stream.readUint16();
			string name = stream.readString();
			string valname = stream.readString();
			
			if(canprint)
				Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: importAlias(" + name + ":" + valname + ")!");
			
			if(valname == "FIXED_DICT")
			{
				KBEDATATYPE_FIXED_DICT datatype = new KBEDATATYPE_FIXED_DICT();
				Byte keysize = stream.readUint8();
				datatype.implementedBy = stream.readString();
					
				while(keysize > 0)
				{
					keysize--;
					
					string keyname = stream.readString();
					UInt16 keyutype = stream.readUint16();
					datatype.dicttype[keyname] = keyutype;
				};
				
				EntityDef.datatypes[name] = datatype;
			}
			else if(valname == "ARRAY")
			{
				UInt16 uitemtype = stream.readUint16();
				KBEDATATYPE_ARRAY datatype = new KBEDATATYPE_ARRAY();
				datatype.type = uitemtype;
				EntityDef.datatypes[name] = datatype;
			}
			else
			{
				KBEDATATYPE_BASE val = null;
				EntityDef.datatypes.TryGetValue(valname, out val);
				EntityDef.datatypes[name] = val;
			}
	
			EntityDef.iddatatypes[utype] = EntityDef.datatypes[name];
			EntityDef.datatype2id[name] = EntityDef.datatype2id[valname];
		}
			
		public void Client_onImportClientEntityDef(MemoryStream stream)
		{
			UInt16 aliassize = stream.readUint16();
			Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: importAlias(size=" + aliassize + ")!");
			
			while(aliassize > 0)
			{
				aliassize--;
				createDataTypeFromStream(stream, true);
			};
		
			foreach(string datatype in EntityDef.datatypes.Keys)
			{
				if(EntityDef.datatypes[datatype] != null)
				{
					EntityDef.datatypes[datatype].bind();
				}
			}
			
			while(stream.opsize() > 0)
			{
				string scriptmethod_name = stream.readString();
				UInt16 scriptUtype = stream.readUint16();
				UInt16 propertysize = stream.readUint16();
				UInt16 methodsize = stream.readUint16();
				UInt16 base_methodsize = stream.readUint16();
				UInt16 cell_methodsize = stream.readUint16();
				
				Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: import(" + scriptmethod_name + "), propertys(" + propertysize + "), " +
						"clientMethods(" + methodsize + "), baseMethods(" + base_methodsize + "), cellMethods(" + cell_methodsize + ")!");
				
				
				ScriptModule module = new ScriptModule(scriptmethod_name);
				EntityDef.moduledefs[scriptmethod_name] = module;
				EntityDef.idmoduledefs[scriptUtype] = module;
				
				Dictionary<string, Property> defpropertys = new Dictionary<string, Property>();
				Entity.alldefpropertys.Add(scriptmethod_name, defpropertys);
				
				Type Class = module.script;
				
				while(propertysize > 0)
				{
					propertysize--;
					
					UInt16 properUtype = stream.readUint16();
					Int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					string defaultValStr = stream.readString();
					KBEDATATYPE_BASE utype = EntityDef.iddatatypes[stream.readUint16()];
					
					System.Reflection.MethodInfo setmethod = null;
					
					if(Class != null)
					{
						setmethod = Class.GetMethod("set_" + name);
					}
					
					Property savedata = new Property();
					savedata.name = name;
					savedata.properUtype = properUtype;
					savedata.aliasID = ialiasID;
					savedata.defaultValStr = defaultValStr;
					savedata.utype = utype;
					savedata.setmethod = setmethod;
					
					module.propertys[name] = savedata;
					
					if(ialiasID >= 0)
					{
						module.usePropertyDescrAlias = true;
						module.idpropertys[(UInt16)ialiasID] = savedata;
					}
					else
					{
						module.usePropertyDescrAlias = false;
						module.idpropertys[properUtype] = savedata;
					}

					Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), property(" + name + "/" + properUtype + ").");
				};
				
				while(methodsize > 0)
				{
					methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
					Int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					
					while(argssize > 0)
					{
						argssize--;
						args.Add(EntityDef.iddatatypes[stream.readUint16()]);
					};
					
					Method savedata = new Method();
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
					
					if(Class != null)
						savedata.handler = Class.GetMethod(name);
							
					module.methods[name] = savedata;
					
					if(ialiasID >= 0)
					{
						module.useMethodDescrAlias = true;
						module.idmethods[(UInt16)ialiasID] = savedata;
					}
					else
					{
						module.useMethodDescrAlias = false;
						module.idmethods[methodUtype] = savedata;
					}
					
					Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), method(" + name + ").");
				};
	
				while(base_methodsize > 0)
				{
					base_methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
					Int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					
					while(argssize > 0)
					{
						argssize--;
						args.Add(EntityDef.iddatatypes[stream.readUint16()]);
					};
					
					Method savedata = new Method();
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
					
					module.base_methods[name] = savedata;
					module.idbase_methods[methodUtype] = savedata;
					
					Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), base_method(" + name + ").");
				};
				
				while(cell_methodsize > 0)
				{
					cell_methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
					Int16 ialiasID = stream.readInt16();
					string name = stream.readString();
					Byte argssize = stream.readUint8();
					List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
					
					while(argssize > 0)
					{
						argssize--;
						args.Add(EntityDef.iddatatypes[stream.readUint16()]);
					};
					
					Method savedata = new Method();
					savedata.name = name;
					savedata.methodUtype = methodUtype;
					savedata.aliasID = ialiasID;
					savedata.args = args;
				
					module.cell_methods[name] = savedata;
					module.idcell_methods[methodUtype] = savedata;
					Dbg.DEBUG_MSG("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), cell_method(" + name + ").");
				};
				
				if(module.script == null)
				{
					Dbg.ERROR_MSG("KBEngine::Client_onImportClientEntityDef: module(" + scriptmethod_name + ") not found!");
				}
					
				foreach(string name in module.propertys.Keys)
				{
					Property infos = module.propertys[name];
					
					Property newp = new Property();
					newp.name = infos.name;
					newp.properUtype = infos.properUtype;
					newp.aliasID = infos.aliasID;
					newp.utype = infos.utype;
					newp.val = infos.utype.parseDefaultValStr(infos.defaultValStr);
					newp.setmethod = infos.setmethod;
					
					defpropertys.Add(infos.name, newp);
					if(module.script != null && module.script.GetMember(name) == null)
					{
						Dbg.ERROR_MSG(scriptmethod_name + "(" + module.script + "):: property(" + name + ") no defined!");
					}
				};
	
				foreach(string name in module.methods.Keys)
				{
					Method infos = module.methods[name];

					if(module.script != null && module.script.GetMethod(name) == null)
					{
						Dbg.WARNING_MSG(scriptmethod_name + "(" + module.script + "):: method(" + name + ") no implement!");
					}
				};
			}
			
			onImportEntityDefCompleted();

		}
		
		private void onImportEntityDefCompleted()
		{
			Dbg.DEBUG_MSG("KBEngine::onImportEntityDefCompleted: successfully!");
			entitydefImported_ = true;
			login_baseapp(false);
		}

		public string mercuryErr(UInt16 id)
		{
			MercuryErr e;
			
			if(!mercuryErrs.TryGetValue(id, out e))
			{
				return "";
			}
			
			
			return e.name;
		}
	
		public void Client_onImportClientMessages(MemoryStream stream)
		{
			UInt16 msgcount = stream.readUint16();
			
			Dbg.DEBUG_MSG(string.Format("KBEngine::Client_onImportClientMessages: start({0})...", msgcount));
			
			while(msgcount > 0)
			{
				msgcount--;
				
				MessageID msgid = stream.readUint16();
				Int16 msglen = stream.readInt16();
				
				string msgname = stream.readString();
				sbyte argstype = stream.readInt8();
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
					handler = typeof(KBEngineApp).GetMethod(msgname);
					if(handler == null)
					{
						Dbg.WARNING_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: interface({1}/{2}/{3}) no implement!", 
							currserver_, msgname, msgid, msglen));
						handler = null;
					}
					else
					{
						Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages: imported({0}/{1}/{2}) successfully!", 
							msgname, msgid, msglen));
					}
				}
				
				if(msgname.Length > 0)
				{
					Message.messages[msgname] = new Message(msgid, msgname, msglen, argstype, argstypes, handler);
					
					if(!isClientMethod)
						Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
							currserver_, msgname, msgid, msglen));
					
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
					Message msg = new Message(msgid, msgname, msglen, argstype, argstypes, handler);
					
					if(!isClientMethod)
						Dbg.DEBUG_MSG(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
							currserver_, msgname, msgid, msglen));
					
					if(currserver_ == "loginapp")
						Message.loginappMessages[msgid] = msg;
					else
						Message.baseappMessages[msgid] = msg;
				}
			};

			onImportClientMessagesCompleted();
		}
		
		public void onOpenLoginapp_resetpassword()
		{  
			Dbg.DEBUG_MSG("KBEngine::onOpenLoginapp_resetpassword: successfully!");
			currserver_ = "loginapp";
			currstate_ = "resetpassword";
			
			if(!loginappMessageImported_)
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Dbg.DEBUG_MSG("KBEngine::onOpenLoginapp_resetpassword: start importClientMessages ...");
			}
			else
			{
				onImportClientMessagesCompleted();
			}
		}
			
		public bool resetpassword_loginapp(bool noconnect)
		{
			if(noconnect)
			{
				reset();
				if(!networkInterface_.connect(ip, port))
				{
					Dbg.ERROR_MSG(string.Format("KBEngine::resetpassword_loginapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onOpenLoginapp_resetpassword();
				Dbg.DEBUG_MSG(string.Format("KBEngine::resetpassword_loginapp(): connect {0}:{1} is successfylly!", ip, port)); 
			}
			else
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_reqAccountResetPassword"]);
				bundle.writeString(username);
				bundle.send(networkInterface_);
			}
			
			return true;
		}
		
		public void onOpenLoginapp_createAccount()
		{  
			Dbg.DEBUG_MSG("KBEngine::onOpenLoginapp_createAccount: successfully!");
			currserver_ = "loginapp";
			currstate_ = "createAccount";
			
			if(!loginappMessageImported_)
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Dbg.DEBUG_MSG("KBEngine::onOpenLoginapp_createAccount: start importClientMessages ...");
			}
			else
			{
				onImportClientMessagesCompleted();
			}
		}
		
		public bool createAccount_loginapp(bool noconnect)
		{
			if(noconnect)
			{
				reset();
				if(!networkInterface_.connect(ip, port))
				{
					Dbg.ERROR_MSG(string.Format("KBEngine::createAccount_loginapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onOpenLoginapp_createAccount();
				Dbg.DEBUG_MSG(string.Format("KBEngine::createAccount_loginapp(): connect {0}:{1} is successfylly!", ip, port));
			}
			else
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_reqCreateAccount"]);
				bundle.writeString(username);
				bundle.writeString(password);
				bundle.writeBlob(new byte[0]);
				bundle.send(networkInterface_);
			}
			
			return true;
		}
		
		public void bindEMail_baseapp()
		{  
			Bundle bundle = new Bundle();
			bundle.newMessage(Message.messages["Baseapp_reqAccountBindEmail"]);
			bundle.writeInt32(entity_id);
			bundle.writeString(password);
			bundle.writeString("3603661@qq.com");
			bundle.send(networkInterface_);
		}
		
		public void newpassword_baseapp(string oldpassword, string newpassword)
		{
			Bundle bundle = new Bundle();
			bundle.newMessage(Message.messages["Baseapp_reqAccountNewPassword"]);
			bundle.writeInt32(entity_id);
			bundle.writeString(oldpassword);
			bundle.writeString(newpassword);
			bundle.send(networkInterface_);
		}
	
		public void Client_onHelloCB(MemoryStream stream)
		{
			serverVersion_ = stream.readString();
			Int32 ctype = stream.readInt32();
			Dbg.DEBUG_MSG("KBEngine::Client_onHelloCB: verInfo(" + serverVersion_ + "), ctype(" + ctype + ")!");
		}
		
		public void Client_onLoginFailed(MemoryStream stream)
		{
			UInt16 failedcode = stream.readUint16();
			serverdatas_ = stream.readBlob();
			Dbg.DEBUG_MSG("KBEngine::Client_onLoginFailed: failedcode(" + failedcode + "), datas(" + serverdatas_.Length + ")!");
			Event.fire("onLoginFailed", new object[]{failedcode});
		}
		
		public void Client_onLoginSuccessfully(MemoryStream stream)
		{
			var accountName = stream.readString();
			username = accountName;
			ip = stream.readString();
			port = stream.readUint16();
			
			Dbg.DEBUG_MSG("KBEngine::Client_onLoginSuccessfully: accountName(" + accountName + "), addr(" + 
					ip + ":" + port + "), datas(" + serverdatas_.Length + ")!");
			
			serverdatas_ = stream.readBlob();
			login_baseapp(true);
		}
		
		public void Client_onLoginGatewayFailed(UInt16 failedcode)
		{
			Dbg.DEBUG_MSG("KBEngine::Client_onLoginGatewayFailed: failedcode(" + failedcode + ")!");
			Event.fire("onLoginGatewayFailed", new object[]{failedcode});
		}
		
		public void Client_onCreatedProxies(UInt64 rndUUID, Int32 eid, string entityType)
		{
			Dbg.DEBUG_MSG("KBEngine::Client_onCreatedProxies: eid(" + eid + "), entityType(" + entityType + ")!");
			entity_uuid = rndUUID;
			entity_id = eid;
			
			Type runclass = EntityDef.moduledefs[entityType].script;
			if(runclass == null)
				return;
			
			Entity entity = (Entity)Activator.CreateInstance(runclass);
			entity.id = eid;
			entity.classtype = entityType;
			
			entity.baseMailbox = new Mailbox();
			entity.baseMailbox.id = eid;
			entity.baseMailbox.classtype = entityType;
			entity.baseMailbox.type = Mailbox.MAILBOX_TYPE.MAILBOX_TYPE_BASE;
			
			entities[eid] = entity;
			
			entity.__init__();
			
			if(entityType == "Account")
			{
				Event.fire("onLoginSuccessfully", new object[]{rndUUID, eid, ((Account)entity)});
			}
			else
			{
				Event.fire("onAvatarEnterWorld", new object[]{rndUUID, eid, ((Avatar)entity)});
			}
		}
		
		public Entity findEntity(Int32 entityID)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(entityID, out entity))
			{
				return null;
			}
			
			return entity;
		}

		public Int32 getAoiEntityIDFromStream(MemoryStream stream)
		{
			Int32 id = 0;
			if(entityIDAliasIDList.Count > 255)
			{
				id = stream.readInt32();
			}
			else
			{
				byte aliasID = stream.readUint8();
				id = entityIDAliasIDList[aliasID];
			}
			
			return id;
		}
		
		public void Client_onUpdatePropertysOptimized(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			onUpdatePropertys_(eid, stream);
		}
		
		public void Client_onUpdatePropertys(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			onUpdatePropertys_(eid, stream);
		}
		
		public void onUpdatePropertys_(Int32 eid, MemoryStream stream)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				MemoryStream entityMessage = null;
				if(bufferedCreateEntityMessage.TryGetValue(eid, out entityMessage))
				{
					Dbg.ERROR_MSG("KBEngine::Client_onUpdatePropertys: entity(" + eid + ") not found!");
					return;
				}
				
				MemoryStream stream1 = new MemoryStream();
				stream1.wpos = stream.wpos;
				stream1.rpos = stream.rpos - 4;
				Array.Copy(stream.data(), stream1.data(), stream.data().Length);
				bufferedCreateEntityMessage[eid] = stream1;
				return;
			}
			
			ScriptModule sm = EntityDef.moduledefs[entity.classtype];
			Dictionary<UInt16, Property> pdatas = sm.idpropertys;

			while(stream.opsize() > 0)
			{
				UInt16 utype = 0;
				
				if(sm.usePropertyDescrAlias)
				{
					utype = stream.readUint8();
				}
				else
				{
					utype = stream.readUint16();
				}
			
				Property propertydata = pdatas[utype];
				utype = propertydata.properUtype;
				System.Reflection.MethodInfo setmethod = propertydata.setmethod;
				
				object val = propertydata.utype.createFromStream(stream);
				object oldval = entity.getDefinedProptertyByUType(utype);
				
				Dbg.DEBUG_MSG("KBEngine::Client_onUpdatePropertys: " + entity.classtype + "(id=" + eid  + " " + propertydata.name + "=" + val + "), hasSetMethod=" + setmethod + "!");
				
				entity.setDefinedProptertyByUType(utype, val);
				if(setmethod != null)
				{
					setmethod.Invoke(entity, new object[]{oldval});
				}
			}
		}

		public void Client_onRemoteMethodCallOptimized(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			onRemoteMethodCall_(eid, stream);
		}
		
		public void Client_onRemoteMethodCall(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			onRemoteMethodCall_(eid, stream);
		}
	
		public void onRemoteMethodCall_(Int32 eid, MemoryStream stream)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onRemoteMethodCall: entity(" + eid + ") not found!");
				return;
			}
			
			UInt16 methodUtype = 0;

			if(EntityDef.moduledefs[entity.classtype].useMethodDescrAlias)
				methodUtype = stream.readUint8();
			else
				methodUtype = stream.readUint16();
			
			Method methoddata = EntityDef.moduledefs[entity.classtype].idmethods[methodUtype];
			
			Dbg.DEBUG_MSG("KBEngine::Client_onRemoteMethodCall: " + entity.classtype + "." + methoddata.name);
			
			object[] args = new object[methoddata.args.Count];
	
			for(int i=0; i<methoddata.args.Count; i++)
			{
				args[i] = methoddata.args[i].createFromStream(stream);
			}
			
			methoddata.handler.Invoke(entity, args);
		}
			
		public void Client_onEntityEnterWorld(Int32 eid, UInt16 uentityType)
		{
			if(entity_id > 0 && entity_id != eid)
				entityIDAliasIDList.Add(eid);
			
			string entityType = EntityDef.idmoduledefs[uentityType].name;
			Dbg.DEBUG_MSG("KBEngine::Client_onEntityEnterWorld: " + entityType + "(" + eid + "), spaceID(" + KBEngineApp.app.spaceID + ")!");
			
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				MemoryStream entityMessage = null;
				if(!bufferedCreateEntityMessage.TryGetValue(eid, out entityMessage))
				{
					Dbg.ERROR_MSG("KBEngine::Client_onEntityEnterWorld: entity(" + eid + ") not found!");
					return;
				}
				
				Type runclass = EntityDef.moduledefs[entityType].script;
				if(runclass == null)
					return;
				
				entity = (Entity)Activator.CreateInstance(runclass);
				entity.id = eid;
				entity.classtype = entityType;
				
				entity.cellMailbox = new Mailbox();
				entity.cellMailbox.id = eid;
				entity.cellMailbox.classtype = entityType;
				entity.cellMailbox.type = Mailbox.MAILBOX_TYPE.MAILBOX_TYPE_CELL;
				
				entities[eid] = entity;
				
				Client_onUpdatePropertys(entityMessage);
				bufferedCreateEntityMessage.Remove(eid);
				
				entity.__init__();
				entity.enterWorld();
			}
			else
			{
				if(!entity.inWorld)
				{
					entity.cellMailbox = new Mailbox();
					entity.cellMailbox.id = eid;
					entity.cellMailbox.classtype = entityType;
					entity.cellMailbox.type = Mailbox.MAILBOX_TYPE.MAILBOX_TYPE_CELL;

					entity.enterWorld();
				}
			}
		}

		public void Client_onEntityLeaveWorldOptimized(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			KBEngineApp.app.Client_onEntityLeaveWorld(eid);
		}
		
		public void Client_onEntityLeaveWorld(Int32 eid)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onEntityLeaveWorld: entity(" + eid + ") not found!");
				return;
			}
			
			if(entity.inWorld)
				entity.leaveWorld();
			
			entities.Remove(eid);
			
			if(entity_id != eid)
				entityIDAliasIDList.Remove(eid);
		}
		
		public void Client_onEntityEnterSpace(UInt32 spaceID, Int32 eid)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onEntityEnterSpace: entity(" + eid + ") not found!");
				return;
			}
		}
		
		public void Client_onEntityLeaveSpace(UInt32 spaceID, Int32 eid)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onEntityLeaveSpace: entity(" + eid + ") not found!");
				return;
			}
		}
	
		public void Client_onCreateAccountResult(MemoryStream stream)
		{
			UInt16 retcode = stream.readUint16();
			byte[] datas = stream.readBlob();
			
			Event.fire("onCreateAccountResult", new object[]{retcode, datas});
			
			if(retcode != 0)
			{
				Dbg.WARNING_MSG("KBEngine::Client_onCreateAccountResult: " + username + " create is failed! code=" + retcode + "!");
				return;
			}
	
			Dbg.DEBUG_MSG("KBEngine::Client_onCreateAccountResult: " + username + " create is successfully!");
		}
		
		public void updatePlayerToServer()
		{
			TimeSpan span = DateTime.Now - lastUpdateToServerTime_; 
			
			if(span.Milliseconds < 50)
				return;
			
			Entity playerEntity = player();
			if(playerEntity == null || playerEntity.inWorld == false)
				return;
			
			lastUpdateToServerTime_ = System.DateTime.Now;
			
			Vector3 position = playerEntity.position;
			Vector3 direction = playerEntity.direction;
			
			bool posHasChanged = Vector3.Distance(entityLastLocalPos, position) > 0.001f;
			bool dirHasChanged = Vector3.Distance(entityLastLocalDir, direction) > 0.001f;
			
			if(posHasChanged || dirHasChanged)
			{
				entityLastLocalPos = position;
				entityLastLocalDir = direction;
				
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Baseapp_onUpdateDataFromClient"]);
				bundle.writeFloat(position.x);
				bundle.writeFloat(position.y);
				bundle.writeFloat(position.z);

				bundle.writeFloat((float)((double)direction.z / 360 * 6.283185307179586));
				bundle.writeFloat((float)((double)direction.y / 360 * 6.283185307179586));
				bundle.writeFloat((float)((double)direction.x / 360 * 6.283185307179586));
				bundle.writeUint8((Byte)(playerEntity.isOnGound == true ? 1 : 0));
				bundle.send(networkInterface_);
			}
		}
		
		public void addSpaceGeometryMapping(UInt32 spaceID, string respath)
		{
			Dbg.DEBUG_MSG("KBEngine::addSpaceGeometryMapping: spaceID(" + spaceID + "), respath(" + respath + ")!");
			
			spaceID = spaceID;
			spaceResPath = respath;
			Event.fire("addSpaceGeometryMapping", new object[]{spaceResPath});
		}
		
		public void Client_initSpaceData(MemoryStream stream)
		{
			spacedatas.Clear();
			UInt32 spaceID = stream.readUint32();
			
			while(stream.opsize() > 0)
			{
				string key = stream.readString();
				string val = stream.readString();
				Client_setSpaceData(spaceID, key, val);
			}
			
			Dbg.DEBUG_MSG("KBEngine::Client_initSpaceData: spaceID(" + spaceID + "), size(" + spacedatas.Count + ")!");
		}
		
		public void Client_setSpaceData(UInt32 spaceID, string key, string value)
		{
			Dbg.DEBUG_MSG("KBEngine::Client_setSpaceData: spaceID(" + spaceID + "), key(" + key + "), value(" + value + ")!");
			spacedatas[key] = value;
			
			if(key == "_mapping")
				addSpaceGeometryMapping(spaceID, value);
		}

		public void Client_delSpaceData(UInt32 spaceID, string key)
		{
			Dbg.DEBUG_MSG("KBEngine::Client_delSpaceData: spaceID(" + spaceID + "), key(" + key + ")");
			spacedatas.Remove(key);
		}
		
		public string getSpaceData(string key)
		{
			string val = "";
			
			if(!spacedatas.TryGetValue(key, out val))
			{
				return "";
			}
			
			return val;
		}

		public void Client_onEntityDestroyed(Int32 eid)
		{
			Dbg.DEBUG_MSG("KBEngine::Client_onEntityDestroyed: entity(" + eid + ")");
			
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onEntityDestroyed: entity(" + eid + ") not found!");
				return;
			}
			
			if(entity.inWorld)
				entity.leaveWorld();
			
			entities.Remove(eid);
		}
		
		public void Client_onUpdateBasePos(MemoryStream stream)
		{
			entityServerPos.x = stream.readFloat();
			entityServerPos.y = stream.readFloat();
			entityServerPos.z = stream.readFloat();
		}
		
		public void Client_onUpdateBasePosXZ(MemoryStream stream)
		{
			entityServerPos.x = stream.readFloat();
			entityServerPos.z = stream.readFloat();
		}
		
		public void Client_onUpdateData(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onUpdateData: entity(" + eid + ") not found!");
				return;
			}
		}
		
		public void Client_onSetEntityPosAndDir(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::Client_onSetEntityPosAndDir: entity(" + eid + ") not found!");
				return;
			}
			
			entity.position.x = stream.readFloat();
			entity.position.y = stream.readFloat();
			entity.position.z = stream.readFloat();
			
			entity.direction.z = KBEMath.int82angle((SByte)stream.readFloat(), false) * 360 / ((float)System.Math.PI * 2);
			entity.direction.y = KBEMath.int82angle((SByte)stream.readFloat(), false) * 360 / ((float)System.Math.PI * 2);
			entity.direction.x = KBEMath.int82angle((SByte)stream.readFloat(), false) * 360 / ((float)System.Math.PI * 2);
			
			Vector3 position = (Vector3)entity.getDefinedPropterty("position");
			Vector3 direction = (Vector3)entity.getDefinedPropterty("direction");
			
			position.x = entity.position.x;
			position.y = entity.position.y;
			position.z = entity.position.z;
			
			direction.x = entity.direction.x;
			direction.y = entity.direction.y;
			direction.z = entity.direction.z;
			
			Event.fire("set_direction", new object[]{entity});
			Event.fire("set_position", new object[]{entity});
		}
		
		public void Client_onUpdateData_ypr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, y, p, r);
		}
		
		public void Client_onUpdateData_yp(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, y, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_yr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte y = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, y, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_pr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_y(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			float y = stream.readPackY();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, y, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_p(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte p = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_r(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, 0.0f, 0.0f, 0.0f, KBE_FLT_MAX, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xz(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_ypr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], y, p, r);
		}
		
		public void Client_onUpdateData_xz_yp(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], y, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_yr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], y, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xz_pr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_xz_y(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			Vector2 xz = stream.readPackXZ();
			SByte yaw = stream.readInt8();
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], yaw, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_p(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte p = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_r(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
	
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], 0.0f, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xyz(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_ypr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], y, xz[1], yaw, p, r);
		}
		
		public void Client_onUpdateData_xyz_yp(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte p = stream.readInt8();

			_updateVolatileData(eid, xz[0], y, xz[1], yaw, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_yr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], y, xz[1], yaw, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xyz_pr(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_xyz_y(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			_updateVolatileData(eid, xz[0], y, xz[1], yaw, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_p(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte p = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_r(MemoryStream stream)
		{
			Int32 eid = getAoiEntityIDFromStream(stream);
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte r = stream.readInt8();
			
			_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, r);
		}
		
		public void _updateVolatileData(Int32 entityID, float x, float y, float z, float yaw, float pitch, float roll)
		{
			Entity entity = null;

			if(!entities.TryGetValue(entityID, out entity))
			{
				Dbg.ERROR_MSG("KBEngine::_updateVolatileData: entity(" + entityID + ") not found!");
				return;
			}
			
			bool changeDirection = false;
			
			if(roll != KBE_FLT_MAX)
			{
				changeDirection = true;
				entity.direction.x = KBEMath.int82angle((SByte)roll, false) * 360 / ((float)System.Math.PI * 2);
			}

			if(pitch != KBE_FLT_MAX)
			{
				changeDirection = true;
				entity.direction.y = KBEMath.int82angle((SByte)pitch, false) * 360 / ((float)System.Math.PI * 2);
			}
			
			if(yaw != KBE_FLT_MAX)
			{
				changeDirection = true;
				entity.direction.z = KBEMath.int82angle((SByte)yaw, false) * 360 / ((float)System.Math.PI * 2);
			}
			
			if(changeDirection == true)
			{
				Event.fire("set_direction", new object[]{entity});
			}
			
			if(!KBEMath.almostEqual(x + y + z, 0f, 0.000001f))
			{
				Vector3 pos = new Vector3(x + entityServerPos.x, y + entityServerPos.y, z + entityServerPos.z);
				entity.position = pos;
				Event.fire("update_position", new object[]{entity});
			}
		}
		
		public void Client_onStreamDataStarted(Int16 id, UInt32 datasize, string descr)
		{
		}
		
		public void Client_onStreamDataRecv(MemoryStream stream)
		{
		}
		
		public void Client_onStreamDataCompleted(Int16 id)
		{
		}
		
		public void Client_onReqAccountResetPasswordCB(UInt16 failcode)
		{
			if(failcode != 0)
			{
				Dbg.ERROR_MSG("KBEngine::Client_onReqAccountResetPasswordCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Dbg.DEBUG_MSG("KBEngine::Client_onReqAccountResetPasswordCB: " + username + " is successfully!");
		}
		
		public void Client_onReqAccountBindEmailCB(UInt16 failcode)
		{
			if(failcode != 0)
			{
				Dbg.ERROR_MSG("KBEngine::Client_onReqAccountBindEmailCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Dbg.DEBUG_MSG("KBEngine::Client_onReqAccountBindEmailCB: " + username + " is successfully!");
		}
		
		public void Client_onReqAccountNewPasswordCB(UInt16 failcode)
		{
			if(failcode != 0)
			{
				Dbg.ERROR_MSG("KBEngine::Client_onReqAccountNewPasswordCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Dbg.DEBUG_MSG("KBEngine::Client_onReqAccountNewPasswordCB: " + username + " is successfully!");
		}
	}
} 
