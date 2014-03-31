namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
    public class SkillBox 
    {
    	public static SkillBox inst = null;
    		
    	public List<Skill> skills = new List<Skill>();
    	
		public SkillBox()
		{
			inst = this;
		}
		
		public void pull()
		{
			clear();
			
			KBEngine.Entity player = KBEngineApp.app.player();
			if(player != null)
				player.cellCall("requestPull", new object[]{});
		}
		
		public void clear()
		{
			skills.Clear();
		}
		
		public void add(Skill skill)
		{
			for(int i=0; i<skills.Count; i++)
			{
				if(skills[i].id == skill.id)
				{
					Dbg.DEBUG_MSG("SkillBox::add: " + skill.id  + " is exist!");
					return;
				}
			}
			
			skills.Add(skill);
		}
		
		public void remove(Int32 id)
		{
			for(int i=0; i<skills.Count; i++)
			{
				if(skills[i].id == id)
				{
					skills.RemoveAt(i);
					return;
				}
			}
		}
		
		public Skill get(Int32 id)
		{
			for(int i=0; i<skills.Count; i++)
			{
				if(skills[i].id == id)
				{
					return skills[i];
				}
			}
			
			return null;
		}
    }
} 
