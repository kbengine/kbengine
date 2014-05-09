namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class Method 
    {
		public string name = "";
    	public UInt16 methodUtype = 0;
    	public Int16 aliasID = -1;
		public List<KBEDATATYPE_BASE> args = new List<KBEDATATYPE_BASE>();
		public System.Reflection.MethodInfo handler = null;
		
		public Method()
		{
			
		}
    }
    
} 
