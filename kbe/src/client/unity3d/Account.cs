namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	using System.Linq;
	
    public class Account : KBEngine.GameObject 
    {
		public Dictionary<UInt64, Dictionary<string, object>> avatars = new Dictionary<UInt64, Dictionary<string, object>>();
		
		public Account()
		{
		}
		
		public override void __init__()
		{
			baseCall("reqAvatarList", new object[0]);
		}
		
		public void onCreateAvatarResult(Byte retcode, object info)
		{
			if(retcode == 0)
			{
				avatars.Add((UInt64)((Dictionary<string, object>)info)["dbid"], (Dictionary<string, object>)info);
				Debug.Log("Account::onCreateAvatarResult: name=" + (string)((Dictionary<string, object>)info)["name"]);
			}
			else
			{
				Debug.LogError("Account::onCreateAvatarResult: retcode=" + retcode);
			}
		}
		
		public void onReqAvatarList(Dictionary<string, object> infos)
		{
			avatars.Clear();
			
			List<object> listinfos = (List<object>)infos["values"];
				
			Debug.Log("Account::onReqAvatarList: avatarsize=" + listinfos.Count);
			for(int i=0; i< listinfos.Count; i++)
			{
				Dictionary<string, object> info = (Dictionary<string, object>)listinfos[i];
				Debug.Log("Account::onReqAvatarList: name" + i + "=" + (string)info["name"]);
				avatars.Add((UInt64)info["dbid"], info);
			}
			
			selectAvatarGame(avatars.Keys.ToList()[0]);
		}
		
		public void reqCreateAvatar(Byte roleType, string name)
		{
			baseCall("reqCreateAvatar", new object[]{roleType, name});
		}
		
		public void selectAvatarGame(UInt64 dbid)
		{
			baseCall("selectAvatarGame", new object[]{dbid});
		}
    }
} 
