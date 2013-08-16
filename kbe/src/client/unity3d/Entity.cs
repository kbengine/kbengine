namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class Entity 
    {
    	public Int32 id = 0;
		public string classtype = "";
		public Vector3 position = new Vector3(0.0f, 0.0f, 0.0f);
		public Vector3 direction = new Vector3(0.0f, 0.0f, 0.0f);
		public float velocity = 0.0f;
		
		public Mailbox baseMailbox = null;
		public Mailbox cellMailbox = null;
		
		public bool inWorld = false;
		
		public static Dictionary<string, Dictionary<string, Property>> alldefpropertys = 
			new Dictionary<string, Dictionary<string, Property>>();
		
		private Dictionary<string, Property> defpropertys_ = 
			new Dictionary<string, Property>();
		
		private Dictionary<UInt16, Property> iddefpropertys_ = 
			new Dictionary<UInt16, Property>();
		
		public Entity()
		{
			Dictionary<string, Property> datas = alldefpropertys[GetType().Name];
			foreach(Property e in datas.Values)
			{
				Property newp = new Property();
				newp.name = e.name;
				newp.properUtype = e.properUtype;
				newp.utype = e.utype;
				newp.val = e.val;
				newp.setmethod = e.setmethod;
				defpropertys_.Add(e.name, newp);
				iddefpropertys_.Add(e.properUtype, newp);
			}
		}
		
		public object getDefinedPropterty(string name)
		{
			return defpropertys_[name].val;
		}
		
		public void setDefinedPropterty(string name, object val)
		{
			defpropertys_[name].val = val;
		}
		
		public object getDefinedProptertyByUType(UInt16 utype)
		{
			return iddefpropertys_[utype].val;
		}
		
		public void setDefinedProptertyByUType(UInt16 utype, object val)
		{
			iddefpropertys_[utype].val = val;
		}
		
		public virtual void __init__()
		{
		}

		public void baseCall(string methodname, object[] arguments)
		{			
			Method method = EntityDef.moduledefs[classtype].base_methods[methodname];
			UInt16 methodID = method.methodUtype;
			
			if(arguments.Length != method.args.Count)
			{
				Debug.LogError("Entity::baseCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
				return;
			}
			
			baseMailbox.newMail();
			baseMailbox.bundle.writeUint16(methodID);
			
			try
			{
				for(var i=0; i<method.args.Count; i++)
				{
					method.args[i].addToStream(baseMailbox.bundle, arguments[i]);
				}
			}
			catch(Exception e)
			{
				Debug.LogError("Entity::baseCall: args is error(" + e.Message + ")!");  
				baseMailbox.bundle = null;
				return;
			}
			
			baseMailbox.postMail(null);
		}
		
		public void cellCall(string methodname, object[] arguments)
		{
			Method method = EntityDef.moduledefs[classtype].cell_methods[methodname];
			UInt16 methodID = method.methodUtype;
			
			if(arguments.Length != method.args.Count)
			{
				Debug.LogError("Entity::cellCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
				return;
			}
			
			cellMailbox.newMail();
			cellMailbox.bundle.writeUint16(methodID);
			
			try
			{
				for(var i=0; i<method.args.Count; i++)
				{
					method.args[i].addToStream(cellMailbox.bundle, arguments[i]);
				}
			}
			catch(Exception e)
			{
				Debug.LogError("Entity::cellCall: args is error(" + e.Message + ")!");  
				cellMailbox.bundle = null;
				return;
			}
			
			cellMailbox.postMail(null);
		}
	
		public virtual void enterWorld()
		{
			Debug.Log(classtype + "::enterWorld: " + id); 
			inWorld = true;
		}
		
		public virtual void leaveWorld()
		{
			Debug.Log(classtype + "::leaveWorld: " + id); 
			inWorld = false;
		}
    }
    
} 
