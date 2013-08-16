namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class ScriptModule
    {
		public string name;
		
		public Dictionary<string, Property> propertys = new Dictionary<string, Property>();
		public Dictionary<UInt16, Property> idpropertys = new Dictionary<UInt16, Property>();
		
		public Dictionary<string, Method> methods = new Dictionary<string, Method>();
		public Dictionary<string, Method> base_methods = new Dictionary<string, Method>();
		public Dictionary<string, Method> cell_methods = new Dictionary<string, Method>();
		
		public Dictionary<UInt16, Method> idmethods = new Dictionary<UInt16, Method>();
		public Dictionary<UInt16, Method> idbase_methods = new Dictionary<UInt16, Method>();
		public Dictionary<UInt16, Method> idcell_methods = new Dictionary<UInt16, Method>();
		
		public Type script = null;
		
		public ScriptModule(string modulename)
		{
			name = modulename;
			script = Type.GetType("KBEngine." + modulename);
		}
    }
    
} 
