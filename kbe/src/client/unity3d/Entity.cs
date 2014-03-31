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
		
		public bool isOnGound = true;
		
		public object renderObj = null;
		
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
		
		public bool isPlayer()
		{
			return id == KBEngineApp.app.entity_id;
		}
		
		public void addDefinedPropterty(string name, object v)
		{
			Property newp = new Property();
			newp.name = name;
			newp.properUtype = 0;
			newp.val = v;
			newp.setmethod = null;
			defpropertys_.Add(name, newp);
		}
		
		public object getDefinedPropterty(string name)
		{
			Property obj = null;
			if(!defpropertys_.TryGetValue(name, out obj))
			{
				return null;
			}
		
			return defpropertys_[name].val;
		}
		
		public void setDefinedPropterty(string name, object val)
		{
			defpropertys_[name].val = val;
		}
		
		public object getDefinedProptertyByUType(UInt16 utype)
		{
			Property obj = null;
			if(!iddefpropertys_.TryGetValue(utype, out obj))
			{
				return null;
			}
			
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
				Dbg.ERROR_MSG("Entity::baseCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
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
				Dbg.ERROR_MSG("Entity::baseCall: args is error(" + e.Message + ")!");  
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
				Dbg.ERROR_MSG("Entity::cellCall: args(" + (arguments.Length) + "!= " + method.args.Count + ") size is error!");  
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
				Dbg.ERROR_MSG("Entity::cellCall: args is error(" + e.Message + ")!");  
				cellMailbox.bundle = null;
				return;
			}

			cellMailbox.postMail(null);
		}
	
		public virtual void enterWorld()
		{
			Dbg.DEBUG_MSG(classtype + "::enterWorld(" + getDefinedPropterty("uid") + "): " + id); 
			inWorld = true;
			Event.fire("onEnterWorld", new object[]{this});
		}
		
		public virtual void leaveWorld()
		{
			Dbg.DEBUG_MSG(classtype + "::leaveWorld: " + id); 
			inWorld = false;
			Event.fire("onLeaveWorld", new object[]{this});
		}
		
		public virtual void set_HP(object old)
		{
			object v = getDefinedPropterty("HP");
			Dbg.DEBUG_MSG(classtype + "::set_HP: " + old + " => " + v); 
			Event.fire("set_HP", new object[]{this, v});
		}
		
		public virtual void set_MP(object old)
		{
			object v = getDefinedPropterty("MP");
			Dbg.DEBUG_MSG(classtype + "::set_MP: " + old + " => " + v); 
			Event.fire("set_MP", new object[]{this, v});
		}
		
		public virtual void set_HP_Max(object old)
		{
			object v = getDefinedPropterty("HP_Max");
			Dbg.DEBUG_MSG(classtype + "::set_HP_Max: " + old + " => " + v); 
			Event.fire("set_HP_Max", new object[]{this, v});
		}
		
		public virtual void set_MP_Max(object old)
		{
			object v = getDefinedPropterty("MP_Max");
			Dbg.DEBUG_MSG(classtype + "::set_MP_Max: " + old + " => " + v); 
			Event.fire("set_MP_Max", new object[]{this, v});
		}
		
		public virtual void set_level(object old)
		{
			object v = getDefinedPropterty("level");
			Dbg.DEBUG_MSG(classtype + "::set_level: " + old + " => " + v); 
			Event.fire("set_level", new object[]{this, v});
		}
		
		public virtual void set_name(object old)
		{
			object v = getDefinedPropterty("name");
			Dbg.DEBUG_MSG(classtype + "::set_name: " + old + " => " + v); 
			Event.fire("set_name", new object[]{this, v});
		}
		
		public virtual void set_state(object old)
		{
			object v = getDefinedPropterty("state");
			Dbg.DEBUG_MSG(classtype + "::set_state: " + old + " => " + v); 
			Event.fire("set_state", new object[]{this, v});
		}
		
		public virtual void set_subState(object old)
		{
			Dbg.DEBUG_MSG(classtype + "::set_subState: " + getDefinedPropterty("subState")); 
		}
		
		public virtual void set_utype(object old)
		{
			Dbg.DEBUG_MSG(classtype + "::set_utype: " + getDefinedPropterty("utype")); 
		}
		
		public virtual void set_uid(object old)
		{
			Dbg.DEBUG_MSG(classtype + "::set_uid: " + getDefinedPropterty("uid")); 
		}
		
		public virtual void set_spaceUType(object old)
		{
			Dbg.DEBUG_MSG(classtype + "::set_spaceUType: " + getDefinedPropterty("spaceUType")); 
		}
		
		public virtual void set_moveSpeed(object old)
		{
			object v = getDefinedPropterty("moveSpeed");
			Dbg.DEBUG_MSG(classtype + "::set_moveSpeed: " + old + " => " + v); 
			Event.fire("set_moveSpeed", new object[]{this, v});
		}
		
		public virtual void set_modelScale(object old)
		{
			object v = getDefinedPropterty("modelScale");
			Dbg.DEBUG_MSG(classtype + "::set_modelScale: " + old + " => " + v); 
			Event.fire("set_modelScale", new object[]{this, v});
		}
		
		public virtual void set_modelID(object old)
		{
			object v = getDefinedPropterty("modelID");
			Dbg.DEBUG_MSG(classtype + "::set_modelID: " + old + " => " + v); 
			Event.fire("set_modelID", new object[]{this, v});
		}
		
		public virtual void set_forbids(object old)
		{
			Dbg.DEBUG_MSG(classtype + "::set_forbids: " + getDefinedPropterty("forbids")); 
		}
		
		public virtual void set_position(object old)
		{
			Vector3 v = (Vector3)getDefinedPropterty("position");
			position = v;
			Dbg.DEBUG_MSG(classtype + "::set_position: " + old + " => " + v); 
			
			if(isPlayer())
				KBEngineApp.app.entityServerPos = position;
			
			Event.fire("set_position", new object[]{this});
		}

		public virtual void set_direction(object old)
		{
			Vector3 v = (Vector3)getDefinedPropterty("direction");
			
			v.x = v.x * 360 / ((float)System.Math.PI * 2);
			v.y = v.y * 360 / ((float)System.Math.PI * 2);
			v.z = v.z * 360 / ((float)System.Math.PI * 2);
			
			direction = v;
			
			Dbg.DEBUG_MSG(classtype + "::set_direction: " + old + " => " + v); 
			Event.fire("set_direction", new object[]{this});
		}
		
		public virtual void recvDamage(Int32 attackerID, Int32 skillID, Int32 damageType, Int32 damage)
		{
			Dbg.DEBUG_MSG(classtype + "::recvDamage: attackerID=" + attackerID + ", skillID=" + skillID + ", damageType=" + damageType + ", damage=" + damage);
			
			Entity entity = KBEngineApp.app.findEntity(attackerID);

			Event.fire("recvDamage", new object[]{this, entity, skillID, damageType, damage});
		}
		
		public virtual void onJump()
		{
			Dbg.DEBUG_MSG(classtype + "::onJump: " + id);
			Event.fire("otherAvatarOnJump", new object[]{this});
		}
		
		public virtual void onAddSkill(Int32 skillID)
		{
			Dbg.DEBUG_MSG(classtype + "::onAddSkill(" + skillID + ")"); 
			Event.fire("onAddSkill", new object[]{this});
			
			Skill skill = new Skill();
			skill.id = skillID;
			skill.name = skillID + " ";
			switch(skillID)
			{
				case 1:
					break;
				case 1000101:
					skill.canUseDistMax = 20f;
					break;
				case 2000101:
					skill.canUseDistMax = 20f;
					break;
				case 3000101:
					skill.canUseDistMax = 20f;
					break;
				case 4000101:
					skill.canUseDistMax = 20f;
					break;
				case 5000101:
					skill.canUseDistMax = 20f;
					break;
				case 6000101:
					skill.canUseDistMax = 20f;
					break;
				default:
					break;
			};

			SkillBox.inst.add(skill);
		}
		
		public virtual void onRemoveSkill(Int32 skillID)
		{
			Dbg.DEBUG_MSG(classtype + "::onRemoveSkill(" + skillID + ")"); 
			Event.fire("onRemoveSkill", new object[]{this});
			SkillBox.inst.remove(skillID);
		}
    }
    
}
