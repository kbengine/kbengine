namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	
    public class Entity 
    {
    	public Int32 id = 0;
		public string classtype = "";
		public Vector3 position = new Vector3(0.0f, 0.0f, 0.0f);
		public Vector3 direction = new Vector3(0.0f, 0.0f, 0.0f);
		public float velocity = 0.0f;
		
		public Entity()
		{
			
		}
    }
    
} 
