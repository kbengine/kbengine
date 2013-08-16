namespace KBEngine
{
  	using UnityEngine; 
	using System; 
	using System.Collections; 
	using System.Collections.Generic;
	
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
		}
		
		public void onReqAvatarList(List<object> infos)
		{
			avatars.Clear();
			Debug.Log("Account::onReqAvatarList: avatarsize=" + infos.Count);
			for(int i=0; i< infos.Count; i++)
			{
				Dictionary<string, object> info = (Dictionary<string, object>)infos[i];
				Debug.Log("Account::onReqAvatarList: name" + i + "=" + (string)info["name"]);
				avatars.Add((UInt64)info["dbid"], info);
			}
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
