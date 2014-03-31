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
		
		public void relive(Byte type)
		{
			cellCall("relive", new object[]{type});
		}
		
		public bool useTargetSkill(Int32 skillID, Int32 targetID)
		{
			Skill skill = SkillBox.inst.get(skillID);
			if(skill == null)
				return false;

			SCEntityObject scobject = new SCEntityObject(targetID);
			if(skill.validCast(this, scobject))
			{
				skill.use(this, scobject);
			}

			return true;
		}
		
		public void jump()
		{
			cellCall("jump", new object[]{});
		}
		
		public override void enterWorld()
		{
			base.enterWorld();

			if(isPlayer())
			{
				SkillBox.inst.pull();
			}
		}
    }
} 
