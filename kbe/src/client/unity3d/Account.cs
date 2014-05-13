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
			Event.fire("onLoginSuccessfully", new object[]{KBEngineApp.app.entity_uuid, id, this});
			baseCall("reqAvatarList", new object[0]);
		}
		
		public void onCreateAvatarResult(Byte retcode, object info)
		{
			if(retcode == 0)
			{
				avatars.Add((UInt64)((Dictionary<string, object>)info)["dbid"], (Dictionary<string, object>)info);
				Dbg.DEBUG_MSG("Account::onCreateAvatarResult: name=" + (string)((Dictionary<string, object>)info)["name"]);
			}
			else
			{
				Dbg.ERROR_MSG("Account::onCreateAvatarResult: retcode=" + retcode);
			}
			
			Event.fire("onCreateAvatarResult", new object[]{retcode, info});
		}
		
		public void onRemoveAvatar(UInt64 dbid)
		{
			Dbg.DEBUG_MSG("Account::onRemoveAvatar: dbid=" + dbid);
			
			avatars.Remove(dbid);
			Event.fire("onRemoveAvatar", new object[]{dbid});
		}
		
		public void onReqAvatarList(Dictionary<string, object> infos)
		{
			avatars.Clear();
			
			List<object> listinfos = (List<object>)infos["values"];
				
			Dbg.DEBUG_MSG("Account::onReqAvatarList: avatarsize=" + listinfos.Count);
			for(int i=0; i< listinfos.Count; i++)
			{
				Dictionary<string, object> info = (Dictionary<string, object>)listinfos[i];
				Dbg.DEBUG_MSG("Account::onReqAvatarList: name" + i + "=" + (string)info["name"]);
				avatars.Add((UInt64)info["dbid"], info);
			}

			Event.fire("onReqAvatarList", new object[]{});

			if(listinfos.Count == 0)
				return;
			
			// selectAvatarGame(avatars.Keys.ToList()[0]);
		}
		
		public void reqCreateAvatar(Byte roleType, string name)
		{
			Dbg.DEBUG_MSG("Account::reqCreateAvatar: roleType=" + roleType);
			baseCall("reqCreateAvatar", new object[]{roleType, name});
		}

		public void reqRemoveAvatar(string name)
		{
			Dbg.DEBUG_MSG("Account::reqRemoveAvatar: name=" + name);
			baseCall("reqRemoveAvatar", new object[]{name});
		}
		
		public void selectAvatarGame(UInt64 dbid)
		{
			Dbg.DEBUG_MSG("Account::selectAvatarGame: dbid=" + dbid);
			baseCall("selectAvatarGame", new object[]{dbid});
		}
    }
} 
