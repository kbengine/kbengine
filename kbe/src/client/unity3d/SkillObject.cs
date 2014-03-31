namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
	public class SCObject
	{
		public SCObject()
		{
		}
		
		public virtual bool valid(KBEngine.Entity caster)
		{
			return true;
		}
		
		public virtual Vector3 getPosition()
		{
			return Vector3.zero;
		}
	};

	public class SCEntityObject : SCObject
	{
		public Int32 targetID;
		public SCEntityObject(Int32 id)
		{
			targetID = id;
		}
		
		public override bool valid(KBEngine.Entity caster)
		{
			return true;
		}
		
		public override Vector3 getPosition()
		{
			KBEngine.Entity entity = KBEngineApp.app.findEntity(targetID);
			if(entity == null)
				return base.getPosition();
			
			return entity.position;
		}
	};
	
	public class SCPositionObject : SCObject
	{
		public Vector3 targetPos;
		public SCPositionObject(Vector3 position)
		{
			targetPos = position;
		}
		
		public override bool valid(KBEngine.Entity caster)
		{
			return true;
		}
		
		public override Vector3 getPosition()
		{
			return targetPos;
		}
	};
	
	public class SRObject
	{
		public SRObject()
		{
		}
		
		public virtual bool valid(KBEngine.Entity receiver)
		{
			return true;
		}
	};
} 
