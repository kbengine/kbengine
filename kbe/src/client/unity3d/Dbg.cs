using UnityEngine;
using KBEngine;
using System.Collections;
namespace KBEngine
{
public class Dbg {
	public static bool isDebugBuild = false;
	
	public static void DEBUG_MSG(object s)
	{
		if(isDebugBuild == false)
			return;
		
#if UNITY_WEBPLAYER
		Application.ExternalCall("console.log", s); 
#endif
		Debug.Log(s);
	}
	
	public static void WARNING_MSG(object s)
	{
#if UNITY_WEBPLAYER
		Application.ExternalCall("console.log", s);
#endif
		Debug.LogWarning(s);
	}
	
	public static void ERROR_MSG(object s)
	{
#if UNITY_WEBPLAYER
		Application.ExternalCall("console.log", s);
#endif
		Debug.LogError(s);
	}
}
}
