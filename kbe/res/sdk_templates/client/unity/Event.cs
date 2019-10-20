namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Threading;

    /// <summary>
    /// KBE-Plugin fire-out events(KBE => Unity):
    /// </summary>
    public class EventOutTypes
    {
        // ------------------------------------账号相关------------------------------------

        /// <summary>
        /// Create account feedback results.
        /// <para> param1(uint16): retcode. // server_errors</para>
        /// <para> param2(bytes): datas. // If you use third-party account system, the system may fill some of the third-party additional datas. </para>
        /// </summary>
        public const string onCreateAccountResult = "onCreateAccountResult";

        /// <summary>
        // Response from binding account Email request.
        // <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onBindAccountEmail = "onBindAccountEmail";

        /// <summary>
        // Response from a new password request.
        // <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onNewPassword = "onNewPassword";

        /// <summary>
        // Response from a reset password request.
        // <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onResetPassword = "onResetPassword";

        // ------------------------------------连接相关------------------------------------
        /// <summary>
        /// Kicked of the current server.
        /// <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onKicked = "onKicked";

        /// <summary>
        /// Disconnected from the server.
        /// </summary>
        public const string onDisconnected = "onDisconnected";

        /// <summary>
        /// Status of connection server.
        /// <para> param1(bool): success or fail</para>
        /// </summary>
        public const string onConnectionState = "onConnectionState";

        // ------------------------------------logon相关------------------------------------
        /// <summary>
        /// Engine version mismatch.
        /// <para> param1(string): clientVersion
        /// <para> param2(string): serverVersion
        /// </summary>
        public const string onVersionNotMatch = "onVersionNotMatch";

        /// <summary>
        /// script version mismatch.
        /// <para> param1(string): clientScriptVersion
        /// <para> param2(string): serverScriptVersion
        /// </summary>
        public const string onScriptVersionNotMatch = "onScriptVersionNotMatch";

        /// <summary>
        /// Login failed.
        /// <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onLoginFailed = "onLoginFailed";

        /// <summary>
        /// Login to baseapp.
        /// </summary>
        public const string onLoginBaseapp = "onLoginBaseapp";

        /// <summary>
        /// Login baseapp failed.
        /// <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onLoginBaseappFailed = "onLoginBaseappFailed";

        /// <summary>
        /// Relogin to baseapp.
        /// </summary>
        public const string onReloginBaseapp = "onReloginBaseapp";

        /// <summary>
        /// Relogin baseapp success.
        /// </summary>
        public const string onReloginBaseappSuccessfully = "onReloginBaseappSuccessfully";

        /// <summary>
        /// Relogin baseapp failed.
        /// <para> param1(uint16): retcode. // server_errors</para>
        /// </summary>
        public const string onReloginBaseappFailed = "onReloginBaseappFailed";

        // ------------------------------------实体cell相关事件------------------------------------

        /// <summary>
        /// Entity enter the client-world.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string onEnterWorld = "onEnterWorld";

        /// <summary>
        /// Entity leave the client-world.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string onLeaveWorld = "onLeaveWorld";

        /// <summary>
        /// Player enter the new space.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string onEnterSpace = "onEnterSpace";

        /// <summary>
        /// Player leave the space.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string onLeaveSpace = "onLeaveSpace";

        /// <summary>
        /// Sets the current position of the entity.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string set_position = "set_position";

        /// <summary>
        /// Sets the current direction of the entity.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string set_direction = "set_direction";

        /// <summary>
        /// The entity position is updated, you can smooth the moving entity to new location.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string updatePosition = "updatePosition";

        /// <summary>
        /// The current space is specified by the geometry mapping.
        /// Popular said is to load the specified Map Resources.
        /// <para> param1(string): resPath</para>
        /// </summary>
        public const string addSpaceGeometryMapping = "addSpaceGeometryMapping";

        /// <summary>
        /// Server spaceData set data.
        /// <para> param1(int32): spaceID</para>
        /// <para> param2(string): key</para>
        /// <para> param3(string): value</para>
        /// </summary>
        public const string onSetSpaceData = "onSetSpaceData";

        /// <summary>
        /// Start downloading data.
        /// <para> param1(int32): rspaceID</para>
        /// <para> param2(string): key</para>
        /// </summary>
        public const string onDelSpaceData = "onDelSpaceData";

        /// <summary>
        /// Triggered when the entity is controlled or out of control.
        /// <para> param1: Entity</para>
        /// <para> param2(bool): isControlled</para>
        /// </summary>
        public const string onControlled = "onControlled";

        /// <summary>
        /// Lose controlled entity.
        /// <para> param1: Entity</para>
        /// </summary>
        public const string onLoseControlledEntity = "onLoseControlledEntity";

        // ------------------------------------数据下载相关------------------------------------
        /// <summary>
        /// Start downloading data.
        /// <para> param1(uint16): resouce id</para>
        /// <para> param2(uint32): data size</para>
        /// <para> param3(string): description</para>
        /// </summary>
        public const string onStreamDataStarted = "onStreamDataStarted";

        /// <summary>
        /// Receive data.
        /// <para> param1(uint16): resouce id</para>
        /// <para> param2(bytes): datas</para>
        /// </summary>
        public const string onStreamDataRecv = "onStreamDataRecv";

        /// <summary>
        /// The downloaded data is completed.
        /// <para> param1(uint16): resouce id</para>
        /// </summary>
        public const string onStreamDataCompleted = "onStreamDataCompleted";
    };

    /// <summary>
    /// KBE-Plugin fire-in events(Unity => KBE):
    /// </summary>
    public class EventInTypes
    {
        /// <summary>
        /// Create new account.
        /// <para> param1(string): accountName</para>
        /// <para> param2(string): password</para>
        /// <para> param3(bytes): datas // Datas by user defined. Data will be recorded into the KBE account database, you can access the datas through the script layer. If you use third-party account system, datas will be submitted to the third-party system.</para>
        /// </summary>
        public const string createAccount = "createAccount";

        /// <summary>
        /// Login to server.
        /// <para> param1(string): accountName</para>
        /// <para> param2(string): password</para>
        /// <para> param3(bytes): datas // Datas by user defined. Data will be recorded into the KBE account database, you can access the datas through the script layer. If you use third-party account system, datas will be submitted to the third-party system.</para>
        /// </summary>
        public const string login = "login";

        /// <summary>
        /// Logout to baseapp, called when exiting the client.
        /// </summary>
        public const string logout = "logout";

        /// <summary>
        /// Relogin to baseapp.
        /// </summary>
        public const string reloginBaseapp = "reloginBaseapp";

        /// <summary>
        /// Reset password.
        /// <para> param1(string): accountName</para>
        /// </summary>
        public const string resetPassword = "resetPassword";

        /// <summary>
        /// Request to set up a new password for the account. Note: account must be online.
        /// <para> param1(string): old_password</para>
        /// <para> param2(string): new_password</para>
        /// </summary>
        public const string newPassword = "newPassword";

        /// <summary>
        /// Request server binding account Email.
        /// <para> param1(string): emailAddress</para>
        /// </summary>
        public const string bindAccountEmail = "bindAccountEmail";
    };

    /// <summary>
    /// 事件模块: KBEngine插件层与Unity3D表现层通过事件来交互，特别是在多线程模式下较方便
	/// </summary>
    public class Event
    {
		public struct Pair
		{
			public object obj;
			public string funcname;
			public System.Reflection.MethodInfo method;
        };
		
		public struct EventObj
		{
			public Pair info;
            public string eventname;
            public object[] args;
		};
		
    	static Dictionary<string, List<Pair>> events_out = new Dictionary<string, List<Pair>>();
		
		public static bool outEventsImmediately = true;

		static LinkedList<EventObj> firedEvents_out = new LinkedList<EventObj>();
		static LinkedList<EventObj> doingEvents_out = new LinkedList<EventObj>();
		
    	static Dictionary<string, List<Pair>> events_in = new Dictionary<string, List<Pair>>();
		
		static LinkedList<EventObj> firedEvents_in = new LinkedList<EventObj>();
		static LinkedList<EventObj> doingEvents_in = new LinkedList<EventObj>();

		static bool _isPauseOut = false;

		public Event()
		{
		}
		
		public static void clear()
		{
			events_out.Clear();
			events_in.Clear();
			clearFiredEvents();
		}

		public static void clearFiredEvents()
		{
			monitor_Enter(events_out);
			firedEvents_out.Clear();
			monitor_Exit(events_out);
			
			doingEvents_out.Clear();
			
			monitor_Enter(events_in);
			firedEvents_in.Clear();
			monitor_Exit(events_in);
			
			doingEvents_in.Clear();
			
			_isPauseOut = false;
		}
		
		public static void pause()
		{
			_isPauseOut = true;
		}

		public static void resume()
		{
			_isPauseOut = false;
			processOutEvents();
		}

		public static bool isPause()
		{
			return _isPauseOut;
		}

		public static void monitor_Enter(object obj)
		{
			if(KBEngineApp.app == null)
				return;
			
			Monitor.Enter(obj);
		}

		public static void monitor_Exit(object obj)
		{
			if(KBEngineApp.app == null)
				return;
			
			Monitor.Exit(obj);
		}
		
		public static bool hasRegisterOut(string eventname)
		{
			return _hasRegister(events_out, eventname);
		}

		public static bool hasRegisterIn(string eventname)
		{
			return _hasRegister(events_in, eventname);
		}
		
		private static bool _hasRegister(Dictionary<string, List<Pair>> events, string eventname)
		{
			bool has = false;
			
			monitor_Enter(events);
			has = events.ContainsKey(eventname);
			monitor_Exit(events);
			
			return has;
		}

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut(string eventname, object obj, string funcname)
		{
			return register(events_out, eventname, obj, funcname);
		}

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut(string eventname, Action handler)
        {
            return registerOut(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut<T1>(string eventname, Action<T1> handler)
        {
            return registerOut(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut<T1, T2>(string eventname, Action<T1, T2> handler)
        {
            return registerOut(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut<T1, T2, T3>(string eventname, Action<T1, T2, T3> handler)
        {
            return registerOut(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
		///	注册监听由kbe插件抛出的事件。(out = kbe->render)
		///	通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		///	事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
        /// </summary>
        public static bool registerOut<T1, T2, T3, T4>(string eventname, Action<T1, T2, T3, T4> handler)
        {
            return registerOut(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn(string eventname, object obj, string funcname)
		{
			return register(events_in, eventname, obj, funcname);
		}

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn(string eventname, Action handler)
        {
            return registerIn(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn<T1>(string eventname, Action<T1> handler)
        {
            return registerIn(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn<T1, T2>(string eventname, Action<T1, T2> handler)
        {
            return registerIn(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn<T1, T2, T3>(string eventname, Action<T1, T2, T3> handler)
        {
            return registerIn(eventname, handler.Target, handler.Method.Name);
        }

        /// <summary>
        /// 注册监听由渲染表现层抛出的事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
        /// </summary>
        public static bool registerIn<T1, T2, T3, T4>(string eventname, Action<T1, T2, T3, T4> handler)
        {
            return registerIn(eventname, handler.Target, handler.Method.Name);
        }

        private static bool register(Dictionary<string, List<Pair>> events, string eventname, object obj, string funcname)
		{
			deregister(events, eventname, obj, funcname);
			List<Pair> lst = null;
			
			Pair pair = new Pair();
			pair.obj = obj;
			pair.funcname = funcname;
			pair.method = obj.GetType().GetMethod(funcname);
			
			if(pair.method == null)
			{
				Dbg.ERROR_MSG("Event::register: " + obj + "not found method[" + funcname + "]");
				return false;
			}
			
			monitor_Enter(events);
			if(!events.TryGetValue(eventname, out lst))
			{
				lst = new List<Pair>();
				lst.Add(pair);
				//Dbg.DEBUG_MSG("Event::register: event(" + eventname + ")!");
				events.Add(eventname, lst);
				monitor_Exit(events);
				return true;
			}
			
			//Dbg.DEBUG_MSG("Event::register: event(" + eventname + ")!");
			lst.Add(pair);
			monitor_Exit(events);
			return true;
		}

		public static bool deregisterOut(string eventname, object obj, string funcname)
		{
            removeFiredEventOut(obj, eventname, funcname);
            return deregister(events_out, eventname, obj, funcname);
		}
  
        public static bool deregisterOut(string eventname, Action handler)
        {
            return deregisterOut(eventname, handler.Target, handler.Method.Name);
        }

        public static bool deregisterIn(string eventname, object obj, string funcname)
		{
            removeFiredEventIn(obj, eventname, funcname);
            return deregister(events_in, eventname, obj, funcname);
		}

        public static bool deregisterIn(string eventname, Action handler)
        {
            return deregisterIn(eventname, handler.Target, handler.Method.Name);
        }

        private static bool deregister(Dictionary<string, List<Pair>> events, string eventname, object obj, string funcname)
		{
			monitor_Enter(events);
			List<Pair> lst = null;
			
			if(!events.TryGetValue(eventname, out lst))
			{
				monitor_Exit(events);
				return false;
			}
			
			for(int i=0; i<lst.Count; i++)
			{
				if(obj == lst[i].obj && lst[i].funcname == funcname)
				{
					//Dbg.DEBUG_MSG("Event::deregister: event(" + eventname + ":" + funcname + ")!");
					lst.RemoveAt(i);
					monitor_Exit(events);
					return true;
				}
			}
			
			monitor_Exit(events);
			return false;
		}

		public static bool deregisterOut(object obj)
		{
            removeAllFiredEventOut(obj);
			return deregister(events_out, obj);
		}

		public static bool deregisterIn(object obj)
		{
            removeAllFiredEventIn(obj);
			return deregister(events_in, obj);
		}
		
		private static bool deregister(Dictionary<string, List<Pair>> events, object obj)
		{
			monitor_Enter(events);
			
			var iter = events.GetEnumerator();
			while (iter.MoveNext())
			{
				List<Pair> lst = iter.Current.Value;
				// 从后往前遍历，以避免中途删除的问题
				for (int i = lst.Count - 1; i >= 0; i--)
				{
					if (obj == lst[i].obj)
					{
						//Dbg.DEBUG_MSG("Event::deregister: event(" + e.Key + ":" + lst[i].funcname + ")!");
						lst.RemoveAt(i);
					}
				}
			}
			
			monitor_Exit(events);
			return true;
		}

        /// <summary>
        /// kbe插件触发事件(out = kbe->render)
		/// 通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
		/// 事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
		/// </summary>
		public static void fireOut(string eventname, params object[] args)
		{
			fire_(events_out, firedEvents_out, eventname, args, outEventsImmediately);
		}

        /// <summary>
        /// 渲染表现层抛出事件(in = render->kbe)
		/// 通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
		/// </summary>
		public static void fireIn(string eventname, params object[] args)
		{
			fire_(events_in, firedEvents_in, eventname, args, false);
		}

        /// <summary>
        /// 触发kbe插件和渲染表现层都能够收到的事件
        /// <summary>
        public static void fireAll(string eventname, params object[] args)
		{
			fire_(events_in, firedEvents_in, eventname, args, false);
			fire_(events_out, firedEvents_out, eventname, args, false);
		}
		
		private static void fire_(Dictionary<string, List<Pair>> events, LinkedList<EventObj> firedEvents, string eventname, object[] args, bool eventsImmediately)
		{
			monitor_Enter(events);
			List<Pair> lst = null;
			
			if(!events.TryGetValue(eventname, out lst))
			{
				//if(events == events_in)
				//	Dbg.WARNING_MSG("Event::fireIn: event(" + eventname + ") not found!");
				//else
				//	Dbg.WARNING_MSG("Event::fireOut: event(" + eventname + ") not found!");
				
				monitor_Exit(events);
				return;
			}
			
			if(eventsImmediately && !_isPauseOut)
			{
				for(int i=0; i<lst.Count; i++)
				{
					Pair info = lst[i];

					try
					{
						info.method.Invoke (info.obj, args);
					}
					catch (Exception e)
					{
						Dbg.ERROR_MSG("Event::fire_: event=" + info.method.DeclaringType.FullName + "::" + info.funcname + "\n" + e.ToString());
					}
				}
			}
			else
			{
				for(int i=0; i<lst.Count; i++)
				{
					EventObj eobj = new EventObj();
					eobj.info = lst[i];
                    eobj.eventname = eventname;
                    eobj.args = args;
					firedEvents.AddLast(eobj);
				}
			}

			monitor_Exit(events);
		}
		
		public static void processOutEvents()
		{
			monitor_Enter(events_out);

			if(firedEvents_out.Count > 0)
			{
				var iter = firedEvents_out.GetEnumerator();
				while (iter.MoveNext())
				{
					doingEvents_out.AddLast(iter.Current);
				}

				firedEvents_out.Clear();
			}

			monitor_Exit(events_out);

			while (doingEvents_out.Count > 0 && !_isPauseOut) 
			{

				EventObj eobj = doingEvents_out.First.Value;

				//Debug.Log("processOutEvents:" + eobj.info.funcname + "(" + eobj.info + ")");
				//foreach(object v in eobj.args)
				//{
				//	Debug.Log("processOutEvents:args=" + v);
				//}
				try
				{
					eobj.info.method.Invoke (eobj.info.obj, eobj.args);
				}
	            catch (Exception e)
	            {
	            	Dbg.ERROR_MSG("Event::processOutEvents: event=" + eobj.info.method.DeclaringType.FullName + "::" + eobj.info.funcname + "\n" + e.ToString());
	            }
            
				if(doingEvents_out.Count > 0)
					doingEvents_out.RemoveFirst();
			}
		}
		
		public static void processInEvents()
		{
			monitor_Enter(events_in);

			if(firedEvents_in.Count > 0)
			{
				var iter = firedEvents_in.GetEnumerator();
				while (iter.MoveNext())
				{
					doingEvents_in.AddLast(iter.Current);
				}

				firedEvents_in.Clear();
			}

			monitor_Exit(events_in);

			while (doingEvents_in.Count > 0) 
			{
				
				EventObj eobj = doingEvents_in.First.Value;
				
				//Debug.Log("processInEvents:" + eobj.info.funcname + "(" + eobj.info + ")");
				//foreach(object v in eobj.args)
				//{
				//	Debug.Log("processInEvents:args=" + v);
				//}
				try
				{
					eobj.info.method.Invoke (eobj.info.obj, eobj.args);
				}
	            catch (Exception e)
	            {
	            	Dbg.ERROR_MSG("Event::processInEvents: event=" + eobj.info.method.DeclaringType.FullName + "::" + eobj.info.funcname + "\n" + e.ToString());
	            }
	            
				if(doingEvents_in.Count > 0)
					doingEvents_in.RemoveFirst();
			}
		}

        public static void removeAllFiredEventIn(object obj)
        {
            removeFiredEvent(firedEvents_in, obj);
        }

        public static void removeAllFiredEventOut(object obj)
        {
            removeFiredEvent(firedEvents_out, obj);
        }

        public static void removeFiredEventIn(object obj, string eventname, string funcname)
        {
            removeFiredEvent(firedEvents_in, obj, eventname, funcname);
        }

        public static void removeFiredEventOut(object obj, string eventname, string funcname)
        {
            removeFiredEvent(firedEvents_out, obj, eventname, funcname);
        }

        public static void removeFiredEvent(LinkedList<EventObj> firedEvents, object obj, string eventname="", string funcname="")
        {
            monitor_Enter(firedEvents);
           
            while(true)
            {
                bool found = false;
                foreach(EventObj eobj in firedEvents)
                {
                    if( ((eventname == "" && funcname == "") || (eventname == eobj.eventname && funcname == eobj.info.funcname))
                        && eobj.info.obj == obj)
                    {
                        firedEvents.Remove(eobj);
                        found = true;
                        break;
                    }
                }

                if (!found)
                    break;
            }
           
            monitor_Exit(firedEvents);
        }
    
    }
} 
