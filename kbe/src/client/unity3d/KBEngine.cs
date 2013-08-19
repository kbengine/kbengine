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
		private byte[] clientdatas_ = new byte[0];
		private string serverVersion_ = "";
		private string clientVersion_ = "0.0.1";
		
		public UInt64 entity_uuid = 0;
		public Int32 entity_id = 0;
		public string entity_type = "";
		
		public Dictionary<Int32, Entity> entities = new Dictionary<Int32, Entity>();
		
		private Dictionary<Int32, MemoryStream> bufferedCreateEntityMessage = new Dictionary<Int32, MemoryStream>(); 
			
		private System.DateTime lastticktime_ = System.DateTime.Now;
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
			clientdatas_ = new byte[0];
			serverVersion_ = "";
			clientVersion_ = "0.0.1";
			
			entity_uuid = 0;
			entity_id = 0;
			entity_type = "";
			
			entities.Clear();
			bufferedCreateEntityMessage.Clear();
			
			lastticktime_ = System.DateTime.Now;
			spaceID = 0;
			spaceResPath = "";
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
			}
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
			
			if(span.Milliseconds >= 100)
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
		
		public void createDataTypeFromStream(MemoryStream stream, bool canprint)
		{
			UInt16 utype = stream.readUint16();
			string name = stream.readString();
			string valname = stream.readString();
			
			if(canprint)
				Debug.Log("KBEngine::Client_onImportClientEntityDef: importAlias(" + name + ":" + valname + ")!");
			
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
			Debug.Log("KBEngine::Client_onImportClientEntityDef: importAlias(size=" + aliassize + ")!");
			
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
				
				Debug.Log("KBEngine::Client_onImportClientEntityDef: import(" + scriptmethod_name + "), propertys(" + propertysize + "), " +
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
					string name = stream.readString();
					string defaultValStr = stream.readString();
					KBEDATATYPE_BASE utype = EntityDef.iddatatypes[stream.readUint16()];
					
					System.Reflection.MethodInfo setmethod = null;
					if(Class != null)
					{
						setmethod = Class.GetType().GetMethod("set_" + name);
					}
					
					Property savedata = new Property();
					savedata.name = name;
					savedata.properUtype = properUtype;
					savedata.defaultValStr = defaultValStr;
					savedata.utype = utype;
					savedata.setmethod = setmethod;
					
					module.propertys[name] = savedata;
					module.idpropertys[properUtype] = savedata;
				
					Debug.Log("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), property(" + name + "/" + properUtype + ").");
				};
				
				while(methodsize > 0)
				{
					methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
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
					savedata.args = args;
					
					if(Class != null)
						savedata.handler = Class.GetMethod(name);
							
					module.methods[name] = savedata;
					module.idmethods[methodUtype] = savedata;
					Debug.Log("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), method(" + name + ").");
				};
	
				while(base_methodsize > 0)
				{
					base_methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
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
					savedata.args = args;
					
					module.base_methods[name] = savedata;
					module.idbase_methods[methodUtype] = savedata;
					
					Debug.Log("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), base_method(" + name + ").");
				};
				
				while(cell_methodsize > 0)
				{
					cell_methodsize--;
					
					UInt16 methodUtype = stream.readUint16();
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
					savedata.args = args;
					
					module.cell_methods[name] = savedata;
					module.idcell_methods[methodUtype] = savedata;
					Debug.Log("KBEngine::Client_onImportClientEntityDef: add(" + scriptmethod_name + "), cell_method(" + name + ").");
				};
				
				if(module.script == null)
				{
					Debug.LogError("KBEngine::Client_onImportClientEntityDef: module(" + scriptmethod_name + ") not found!");
				}
					
				foreach(string name in module.propertys.Keys)
				{
					Property infos = module.propertys[name];
					
					Property newp = new Property();
					newp.name = infos.name;
					newp.properUtype = infos.properUtype;
					newp.utype = infos.utype;
					newp.val = infos.utype.parseDefaultValStr(infos.defaultValStr);
					newp.setmethod = infos.setmethod;
					
					defpropertys.Add(infos.name, newp);
					if(module.script != null && module.script.GetType().GetMember(name) == null)
					{
						Debug.LogError(scriptmethod_name + ":: property(" + name + ") no defined!");
					}
				};
	
				foreach(string name in module.methods.Keys)
				{
					Method infos = module.methods[name];

					if(module.script != null && module.script.GetType().GetMethod(name) == null)
					{
						Debug.LogWarning(scriptmethod_name + ":: method(" + name + ") no implement!");
					}
				};
			}
			
			onImportEntityDefCompleted();

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
					handler = typeof(KBEngineApp).GetMethod(msgname);
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
					
					if(!isClientMethod)
						Debug.Log(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
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
					Message msg = new Message(msgid, msgname, msglen, argstypes, handler);
					
					if(!isClientMethod)
						Debug.Log(string.Format("KBEngine::onImportClientMessages[{0}]: imported({1}/{2}/{3}) successfully!", 
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
			Debug.Log("KBEngine::onOpenLoginapp_resetpassword: successfully!");
			currserver_ = "loginapp";
			currstate_ = "resetpassword";
			
			if(!loginappMessageImported_)
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Debug.Log("KBEngine::onOpenLoginapp_resetpassword: start importClientMessages ...");
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
				if(!networkInterface_.connect(ip, port))
				{
					Debug.LogError(string.Format("KBEngine::resetpassword_loginapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onOpenLoginapp_resetpassword();
				Debug.Log(string.Format("KBEngine::resetpassword_loginapp(): connect {0}:{1} is successfylly!", ip, port)); 
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
			Debug.Log("KBEngine::onOpenLoginapp_createAccount: successfully!");
			currserver_ = "loginapp";
			currstate_ = "createAccount";
			
			if(!loginappMessageImported_)
			{
				Bundle bundle = new Bundle();
				bundle.newMessage(Message.messages["Loginapp_importClientMessages"]);
				bundle.send(networkInterface_);
				Debug.Log("KBEngine::onOpenLoginapp_createAccount: start importClientMessages ...");
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
				if(!networkInterface_.connect(ip, port))
				{
					Debug.LogError(string.Format("KBEngine::createAccount_loginapp(): connect {0}:{1} is error!", ip, port));
					return false;
				}
				
				onOpenLoginapp_createAccount();
				Debug.Log(string.Format("KBEngine::createAccount_loginapp(): connect {0}:{1} is successfylly!", ip, port));
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
		
		public void Client_onCreatedProxies(UInt64 rndUUID, Int32 eid, string entityType)
		{
			Debug.Log("KBEngine::Client_onCreatedProxies: eid(" + eid + "), entityType(" + entityType + ")!");
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
			
			((Account)entity).reqCreateAvatar(1, "kebiao");
		}
		
		public void Client_onUpdatePropertys(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				MemoryStream entityMessage = null;
				if(bufferedCreateEntityMessage.TryGetValue(eid, out entityMessage))
				{
					Debug.LogError("KBEngine::Client_onUpdatePropertys: entity(" + eid + ") not found!");
					return;
				}
				
				MemoryStream stream1 = new MemoryStream();
				stream1.wpos = stream.wpos;
				stream1.rpos = stream.rpos - 4;
				Array.Copy(stream.data(), stream1.data(), stream.data().Length);
				bufferedCreateEntityMessage[eid] = stream1;
				return;
			}
			
			Dictionary<UInt16, Property> pdatas = EntityDef.moduledefs[entity.classtype].idpropertys;
			while(stream.opsize() > 0)
			{
				UInt16 utype = stream.readUint16();
				Property propertydata = pdatas[utype];
				System.Reflection.MethodInfo setmethod = propertydata.setmethod;
				
				object val = propertydata.utype.createFromStream(stream);
				object oldval = entity.getDefinedProptertyByUType(utype);
				
				Debug.Log("KBEngine::Client_onUpdatePropertys: " + entity.classtype + "(id=" + eid  + " " + propertydata.name + ", val=" + val + ")!");
				
				entity.setDefinedProptertyByUType(utype, val);
				if(setmethod != null)
				{
					setmethod.Invoke(entity, new object[]{oldval});
				}
			}
		}

		public void Client_onRemoteMethodCall(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onRemoteMethodCall: entity(" + eid + ") not found!");
				return;
			}
			
			UInt16 methodUtype = stream.readUint16();
			Method methoddata = EntityDef.moduledefs[entity.classtype].idmethods[methodUtype];
			
			Debug.Log("KBEngine::Client_onRemoteMethodCall: " + entity.classtype + "." + methoddata.name);
			
			object[] args = new object[methoddata.args.Count];
	
			for(int i=0; i<methoddata.args.Count; i++)
			{
				args[i] = methoddata.args[i].createFromStream(stream);
			}
			
			methoddata.handler.Invoke(entity, args);
		}
			
		public void Client_onEntityEnterWorld(Int32 eid, UInt16 uentityType, UInt32 spaceID)
		{
			string entityType = EntityDef.idmoduledefs[uentityType].name;
			Debug.Log("KBEngine::Client_onEntityEnterWorld: " + entityType + "(" + eid + "), spaceID(" + spaceID + ")!");
			
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				MemoryStream entityMessage = null;
				if(!bufferedCreateEntityMessage.TryGetValue(eid, out entityMessage))
				{
					Debug.LogError("KBEngine::Client_onEntityEnterWorld: entity(" + eid + ") not found!");
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
					entity.enterWorld();
				}
			}
		}
		
		public void Client_onEntityLeaveWorld(Int32 eid, UInt32 spaceID)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onEntityLeaveWorld: entity(" + eid + ") not found!");
				return;
			}
			
			if(entity.inWorld)
				entity.leaveWorld();
			
			entities.Remove(eid);
		}
		
		public void Client_onEntityEnterSpace(UInt32 spaceID, Int32 eid)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onEntityEnterSpace: entity(" + eid + ") not found!");
				return;
			}
		}
		
		public void Client_onEntityLeaveSpace(UInt32 spaceID, Int32 eid)
		{
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onEntityLeaveSpace: entity(" + eid + ") not found!");
				return;
			}
		}
	
		public void Client_onSetEntityPosAndDir(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onSetEntityPosAndDir: entity(" + eid + ") not found!");
				return;
			}
			
			Vector3 position = (Vector3)entity.getDefinedPropterty("position");
			Vector3 direction = (Vector3)entity.getDefinedPropterty("direction");
			
			position.x = stream.readFloat();
			position.y = stream.readFloat();
			position.z = stream.readFloat();
			
			direction.x = stream.readFloat();
			direction.y = stream.readFloat();
			direction.z = stream.readFloat();
		}
	
		public void Client_onCreateAccountResult(MemoryStream stream)
		{
			UInt16 retcode = stream.readUint16();
			byte[] datas = stream.readBlob();
			
			if(retcode != 0)
			{
				Debug.LogError("KBEngine::Client_onCreateAccountResult: " + username + " create is failed! code=" + retcode + "!");
				return;
			}
	
			Debug.Log("KBEngine::Client_onCreateAccountResult: " + username + " create is successfully!");
		}
		
		public void updatePlayerToServer()
		{
			Entity playerEntity = player();
			if(playerEntity == null || playerEntity.inWorld == false)
				return;
			
			Vector3 position = (Vector3)playerEntity.getDefinedPropterty("position");
			Vector3 direction = (Vector3)playerEntity.getDefinedPropterty("direction");
			
			Bundle bundle = new Bundle();
			bundle.newMessage(Message.messages["Baseapp_onUpdateDataFromClient"]);
			bundle.writeFloat(position.x);
			bundle.writeFloat(position.y);
			bundle.writeFloat(position.z);
			bundle.writeFloat(direction.z);
			bundle.writeFloat(direction.y);
			bundle.writeFloat(direction.x);
			bundle.send(networkInterface_);
		}
		
		public void Client_addSpaceGeometryMapping(UInt32 spaceID, string respath)
		{
			Debug.Log("KBEngine::Client_addSpaceGeometryMapping: spaceID(" + spaceID + "), respath(" + respath + ")!");
			
			spaceID = spaceID;
			spaceResPath = respath;
		}
		
		public void Client_onUpdateBasePos(MemoryStream stream)
		{
			Vector3 pos = new Vector3(stream.readFloat(), stream.readFloat(), stream.readFloat());
		}
		
		public void Client_onUpdateData(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			Entity entity = null;
			
			if(!entities.TryGetValue(eid, out entity))
			{
				Debug.LogError("KBEngine::Client_onUpdateData: entity(" + eid + ") not found!");
				return;
			}
		}
		
		public void Client_onUpdateData_ypr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, y, p, r);
		}
		
		public void Client_onUpdateData_yp(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, y, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_yr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte y = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, y, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_pr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_y(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			float y = stream.readPackY();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, y, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_p(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_r(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, 0.0, 0.0, 0.0, KBE_FLT_MAX, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xz(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_ypr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], y, p, r);
		}
		
		public void Client_onUpdateData_xz_yp(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], y, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_yr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], y, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xz_pr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_xz_y(MemoryStream stream)
		{
			var eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte y = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], y, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_p(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xz_r(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
	
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], 0.0, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xyz(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_ypr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], yaw, p, r);
		}
		
		public void Client_onUpdateData_xyz_yp(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], yaw, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_yr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], yaw, KBE_FLT_MAX, r);
		}
		
		public void Client_onUpdateData_xyz_pr(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte p = stream.readInt8();
			SByte r = stream.readInt8();
			
			//_updateVolatileData(eid, x, y, z, KBE_FLT_MAX, p, r);
		}
		
		public void Client_onUpdateData_xyz_y(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte yaw = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], yaw, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_p(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], KBE_FLT_MAX, p, KBE_FLT_MAX);
		}
		
		public void Client_onUpdateData_xyz_r(MemoryStream stream)
		{
			Int32 eid = stream.readInt32();
			
			Vector2 xz = stream.readPackXZ();
			float y = stream.readPackY();
			
			SByte p = stream.readInt8();
			
			//_updateVolatileData(eid, xz[0], y, xz[1], r, KBE_FLT_MAX, KBE_FLT_MAX);
		}
		
		public void _updateVolatileData(Int32 entityID, float x, float y, float z, float yaw, float pitch, float roll)
		{
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
				Debug.LogError("KBEngine::Client_onReqAccountResetPasswordCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Debug.Log("KBEngine::Client_onReqAccountResetPasswordCB: " + username + " is successfully!");
		}
		
		public void Client_onReqAccountBindEmailCB(UInt16 failcode)
		{
			if(failcode != 0)
			{
				Debug.LogError("KBEngine::Client_onReqAccountBindEmailCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Debug.Log("KBEngine::Client_onReqAccountBindEmailCB: " + username + " is successfully!");
		}
		
		public void Client_onReqAccountNewPasswordCB(UInt16 failcode)
		{
			if(failcode != 0)
			{
				Debug.LogError("KBEngine::Client_onReqAccountNewPasswordCB: " + username + " is failed! code=" + failcode + "!");
				return;
			}
	
			Debug.Log("KBEngine::Client_onReqAccountNewPasswordCB: " + username + " is successfully!");
		}
	}
} 
