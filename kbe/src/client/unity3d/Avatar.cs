namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class Avatar : KBEngine.GameObject   
    {
    	public CombatImpl combat = null;
    	
		public Avatar()
		{
		}
		
		public override void __init__()
		{
			combat = new CombatImpl(this);
		}
    }
} 
