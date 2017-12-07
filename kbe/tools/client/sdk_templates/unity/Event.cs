namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Threading;
	
	/*
		事件模块
		KBEngine插件层与Unity3D表现层通过事件来交互
	*/
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
			public object[] args;
		};
		
    	static Dictionary<string, List<Pair>> events_out = new Dictionary<string, List<Pair>>();
		
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
		
		/*
			注册监听由kbe插件抛出的事件。(out = kbe->render)
			通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
			事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
		*/
		public static bool registerOut(string eventname, object obj, string funcname)
		{
			return register(events_out, eventname, obj, funcname);
		}

		/*
			注册监听由渲染表现层抛出的事件(in = render->kbe)
			通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
		*/
		public static bool registerIn(string eventname, object obj, string funcname)
		{
			return register(events_in, eventname, obj, funcname);
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
			return deregister(events_out, eventname, obj, funcname);
		}

		public static bool deregisterIn(string eventname, object obj, string funcname)
		{
			return deregister(events_in, eventname, obj, funcname);
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
			return deregister(events_out, obj);
		}

		public static bool deregisterIn(object obj)
		{
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

		/*
			kbe插件触发事件(out = kbe->render)
			通常由渲染表现层来注册, 例如：监听角色血量属性的变化， 如果UI层注册这个事件，
			事件触发后就可以根据事件所附带的当前血量值来改变角色头顶的血条值。
		*/
		public static void fireOut(string eventname, params object[] args)
		{
			fire_(events_out, firedEvents_out, eventname, args);
		}

		/*
			渲染表现层抛出事件(in = render->kbe)
			通常由kbe插件层来注册， 例如：UI层点击登录， 此时需要触发一个事件给kbe插件层进行与服务端交互的处理。
		*/
		public static void fireIn(string eventname, params object[] args)
		{
			fire_(events_in, firedEvents_in, eventname, args);
		}

		/*
			触发kbe插件和渲染表现层都能够收到的事件
		*/
		public static void fireAll(string eventname, params object[] args)
		{
			fire_(events_in, firedEvents_in, eventname, args);
			fire_(events_out, firedEvents_out, eventname, args);
		}
		
		private static void fire_(Dictionary<string, List<Pair>> events, LinkedList<EventObj> firedEvents, string eventname, object[] args)
		{
			monitor_Enter(events);
			List<Pair> lst = null;
			
			if(!events.TryGetValue(eventname, out lst))
			{
				if(events == events_in)
					Dbg.WARNING_MSG("Event::fireIn: event(" + eventname + ") not found!");
				else
					Dbg.WARNING_MSG("Event::fireOut: event(" + eventname + ") not found!");
				
				monitor_Exit(events);
				return;
			}
			
			for(int i=0; i<lst.Count; i++)
			{
				EventObj eobj = new EventObj();
				eobj.info = lst[i];
				eobj.args = args;
				firedEvents.AddLast(eobj);
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
	            	Dbg.ERROR_MSG("Event::processOutEvents: event=" + eobj.info.funcname + "\n" + e.ToString());
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
	            	Dbg.ERROR_MSG("Event::processInEvents: event=" + eobj.info.funcname + "\n" + e.ToString());
	            }
	            
				if(doingEvents_in.Count > 0)
					doingEvents_in.RemoveFirst();
			}
		}
	
    }
} 
