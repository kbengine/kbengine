namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	
    public class Property 
    {
		public string name = "";
    	public KBEDATATYPE_BASE utype = null;
		public UInt16 properUtype = 0;
		public Int16 aliasID = -1;
		
		public string defaultValStr = "";
		public System.Reflection.MethodInfo setmethod = null;
		
		public object val = null;
		
		public Property()
		{
			
		}
    }
    
} 
