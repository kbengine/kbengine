﻿using UnityEngine;
using System;
using System.Collections;
using KBEngine;

/*
	可以理解为插件的入口模块
	在这个入口中安装了需要监听的事件(installEvents)，同时初始化KBEngine(initKBEngine)
*/
	
public class KBEMain : MonoBehaviour 
{
	public KBEngineApp gameapp = null;
	
	// 在unity3d界面中可见选项
	public DEBUGLEVEL debugLevel = DEBUGLEVEL.DEBUG;
	public bool isMultiThreads = false;
	public string ip = "127.0.0.1";
	public int port = @{KBE_LOGIN_PORT};
	public KBEngineApp.CLIENT_TYPE clientType = KBEngineApp.CLIENT_TYPE.CLIENT_TYPE_MINI;
	public KBEngineApp.NETWORK_ENCRYPT_TYPE networkEncryptType = KBEngineApp.NETWORK_ENCRYPT_TYPE.ENCRYPT_TYPE_NONE;
	public int syncPlayerMS = 1000 / @{KBE_UPDATEHZ};
	public int threadUpdateHZ = @{KBE_UPDATEHZ} * 2;
	public int serverHeartbeatTick = @{KBE_SERVER_EXTERNAL_TIMEOUT};
	public int SEND_BUFFER_MAX = (int)KBEngine.NetworkInterface.TCP_PACKET_MAX;
	public int RECV_BUFFER_MAX = (int)KBEngine.NetworkInterface.TCP_PACKET_MAX;
	public bool useAliasEntityID = @{KBE_USE_ALIAS_ENTITYID};
	public bool isOnInitCallPropertysSetMethods = true;

	public bool automaticallyUpdateSDK = true;

	protected virtual void Awake() 
	 {
		DontDestroyOnLoad(transform.gameObject);
	 }
 
	// Use this for initialization
	protected virtual void Start () 
	{
		MonoBehaviour.print("clientapp::start()");
		installEvents();
		initKBEngine();
	}
	
	public virtual void installEvents()
	{
        KBEngine.Event.registerOut(EventOutTypes.onVersionNotMatch, this, "onVersionNotMatch");
        KBEngine.Event.registerOut(EventOutTypes.onScriptVersionNotMatch, this, "onScriptVersionNotMatch");
	}
	
	public void onVersionNotMatch(string verInfo, string serVerInfo)
	{
#if UNITY_EDITOR
		if(automaticallyUpdateSDK)
			gameObject.AddComponent<ClientSDKUpdater>();
#endif
	}

	public void onScriptVersionNotMatch(string verInfo, string serVerInfo)
	{
#if UNITY_EDITOR
		if(automaticallyUpdateSDK)
			gameObject.AddComponent<ClientSDKUpdater>();
#endif
	}

	public virtual void initKBEngine()
	{
		// 如果此处发生错误，请查看 Assets\Scripts\kbe_scripts\if_Entity_error_use______git_submodule_update_____kbengine_plugins_______open_this_file_and_I_will_tell_you.cs

		Dbg.debugLevel = debugLevel;

		KBEngineArgs args = new KBEngineArgs();
		
		args.ip = ip;
		args.port = port;
		args.clientType = clientType;
        args.networkEncryptType = networkEncryptType;
        args.syncPlayerMS = syncPlayerMS;
		args.threadUpdateHZ = threadUpdateHZ;
		args.serverHeartbeatTick = serverHeartbeatTick / 2;
		args.useAliasEntityID = useAliasEntityID;
		args.isOnInitCallPropertysSetMethods = isOnInitCallPropertysSetMethods;

		args.SEND_BUFFER_MAX = (UInt32)SEND_BUFFER_MAX;
		args.RECV_BUFFER_MAX = (UInt32)RECV_BUFFER_MAX;
		
		args.isMultiThreads = isMultiThreads;
		
		if(isMultiThreads)
			gameapp = new KBEngineAppThread(args);
		else
			gameapp = new KBEngineApp(args);
	}
	
	protected virtual void OnDestroy()
	{
		MonoBehaviour.print("clientapp::OnDestroy(): begin");
        if (KBEngineApp.app != null)
        {
            KBEngineApp.app.destroy();
            KBEngineApp.app = null;
        }
		KBEngine.Event.clear();
		MonoBehaviour.print("clientapp::OnDestroy(): end");
	}
	
	protected virtual void FixedUpdate () 
	{
		KBEUpdate();
	}

	public virtual void KBEUpdate()
	{
		// 单线程模式必须自己调用
		if(!isMultiThreads)
			gameapp.process();
		
		KBEngine.Event.processOutEvents();
	}
}
