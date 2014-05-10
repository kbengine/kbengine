namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Threading;
	
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
		
    	public static Dictionary<string, List<Pair>> events = new Dictionary<string, List<Pair>>();
		
		public static List<EventObj> firedEvents = new List<EventObj>();
		private static List<EventObj> doingEvents = new List<EventObj>();
		
		public Event()
		{
		}
		
		public static bool register(string eventname, object obj, string funcname)
		{
			deregister(eventname, obj, funcname);
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
			
			Monitor.Enter(events);
			if(!events.TryGetValue(eventname, out lst))
			{
				lst = new List<Pair>();
				lst.Add(pair);
				Dbg.DEBUG_MSG("Event::register: event(" + eventname + ")!");
				events.Add(eventname, lst);
				Monitor.Exit(events);
				return true;
			}
			
			Dbg.DEBUG_MSG("Event::register: event(" + eventname + ")!");
			lst.Add(pair);
			Monitor.Exit(events);
			return true;
		}
		
		public static bool deregister(string eventname, object obj, string funcname)
		{
			Monitor.Enter(events);
			List<Pair> lst = null;
			
			if(!events.TryGetValue(eventname, out lst))
			{
				Monitor.Exit(events);
				return false;
			}
			
			for(int i=0; i<lst.Count; i++)
			{
				if(obj == lst[i].obj && lst[i].funcname == funcname)
				{
					Dbg.DEBUG_MSG("Event::deregister: event(" + eventname + ":" + funcname + ")!");
					lst.RemoveAt(i);
					Monitor.Exit(events);
					return true;
				}
			}
			
			Monitor.Exit(events);
			return false;
		}

		public static bool deregister(object obj)
		{
			Monitor.Enter(events);
			
			foreach(KeyValuePair<string, List<Pair>> e in events)
			{
				List<Pair> lst = e.Value;
__RESTART_REMOVE:
				for(int i=0; i<lst.Count; i++)
				{
					if(obj == lst[i].obj)
					{
						Dbg.DEBUG_MSG("Event::deregister: event(" + e.Key + ":" + lst[i].funcname + ")!");
						lst.RemoveAt(i);
						goto __RESTART_REMOVE;
					}
				}
			}
			
			Monitor.Exit(events);
			return true;
		}
		
		public static void fire(string eventname, object[] args)
		{
			Monitor.Enter(events);
			List<Pair> lst = null;
			
			if(!events.TryGetValue(eventname, out lst))
			{
				Dbg.ERROR_MSG("Event::fire: event(" + eventname + ") not found!");
				Monitor.Exit(events);
				return;
			}
			
			for(int i=0; i<lst.Count; i++)
			{
				EventObj eobj = new EventObj();
				eobj.info = lst[i];
				eobj.args = args;
				firedEvents.Add(eobj);
			}
			
			Monitor.Exit(events);
		}
		
		public static void processEventsMainThread()
		{
			Monitor.Enter(events);

			if(firedEvents.Count > 0)
			{
				foreach(EventObj evt in firedEvents)
				{
					doingEvents.Add(evt);
				}

				firedEvents.Clear();
			}

			Monitor.Exit(events);
			
			for(int i=0; i<doingEvents.Count; i++)
			{
				EventObj eobj = doingEvents[i];
				
				//Debug.Log("processEventsMainThread:" + eobj.info.funcname + "(" + eobj.info + ")");
				//foreach(object v in eobj.args)
				//{
				//	Debug.Log("processEventsMainThread:args=" + v);
				//}
				
				eobj.info.method.Invoke(eobj.info.obj, eobj.args);
			}
			
			doingEvents.Clear();
		}
    }
    
} 
