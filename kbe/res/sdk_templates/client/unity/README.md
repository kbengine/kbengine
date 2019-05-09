kbengine_unity3d_plugins
========================

Usage
---------------------

	1: Generate client plugins through projects
		1: Double click to run kbengine\*assets\gensdk.bat
		2: Copy kbengine_unity3d_plugins to {UnityProjectName}\Assets\Plugins
		
	2: Create clientapp.cs
		using KBEngine;
		public class clientapp : KBEMain 
		{
		}

	3: Implment the KBE defined entity (including the client part)
		See: kbengine\kbengine_demos_assets\scripts\entities.xml->hasClient="true" need to implment
			<Account hasClient="true"></Account>
			<Monster hasClient="true"></Monster>
			<Gate hasClient="true"></Gate>
			<Space/>

			public class Account : AccountBase
			{
				// entity initialization
				public override void __init__()
				{
				}
			}

		Call entity server method
			Account.cs: baseEntityCall.reqAvatarList();
			Avatar.cs: cellEntityCall.relive(reliveType);

		Reference: https://github.com/kbengine/kbengine/issues/532

	4: Monitor KBE-plugins event
		For example:
			public class UI : MonoBehaviour
			{
				void Start () 
				{
					KBEngine.Event.registerOut("onConnectionState", this, "onConnectionState");
				}

				public void onConnectionState(bool success)
				{
					// KBE-plugins event fired
				}
			}

	5: Fire events to the KBE-plugins
		For example:
			KBEngine.Event.fireIn("login", "stringAccount", "stringPasswd", System.Text.Encoding.UTF8.GetBytes("kbengine_unity3d_demo"));



KBE-Plugin fire-out events(KBE => Unity):
---------------------

	Entity events:
		onEnterWorld
			Description: 
				Entity enter the client-world.

			Event-datas: 
				Enity
				

		onLeaveWorld
			Description: 
				Entity leave the client-world.

			Event-datas: 
				Enity

		onEnterSpace
			Description: 
				Player enter the new space.

			Event-datas: 
				Enity

		onLeaveSpace
			Description: 
				Player enter the space.

			Event-datas: 
				Enity

		onCreateAccountResult
			Description: 
				Create account feedback results.

			Event-datas: 
				uint16: retcode
					http://kbengine.org/docs/configuration/server_errors.html

				bytes: datas
					If you use third-party account system, the system may fill some of the third-party additional datas.

		onControlled
			Description: 
				Triggered when the entity is controlled or out of control.

			Event-datas: 
				Enity
				bool: isControlled

		onLoseControlledEntity
			Description: 
				Lose controlled entity.

			Event-datas: 
				Enity

		set_position
			Description: 
				Sets the current position of the entity.

			Event-datas: 
				Enity

		set_direction
			Description: 
				Sets the current direction of the entity.

			Event-datas: 
				Enity

		updatePosition
			Description: 
				The entity position is updated, you can smooth the moving entity to new location.

			Event-datas: 
				Enity

	Protocol events:
		onVersionNotMatch
			Description: 
				Engine version mismatch.

			Event-datas: 
				string: clientVersion
				string: serverVersion

		onScriptVersionNotMatch
			Description: 
				script version mismatch.

			Event-datas: 
				string: clientScriptVersion
				string: serverScriptVersion

		Loginapp_importClientMessages
			Description: 
				Importing the message protocol for loginapp and client.

			Event-datas: 
				No datas.

		Baseapp_importClientMessages
			Description: 
				Importing the message protocol for baseapp and client.

			Event-datas: 
				No datas.

		Baseapp_importClientEntityDef
			Description: 
				Protocol description for importing entities.

			Event-datas: 
				No datas.

	Login and Logout status:
		onLoginBaseapp
			Description: 
				Login to baseapp.

			Event-datas: 
				No datas.

		onReloginBaseapp
			Description: 
				Relogin to baseapp.

			Event-datas: 
				No datas.

		onKicked
			Description: 
				Kicked of the current server.

			Event-datas: 
				uint16: retcode
					http://kbengine.org/docs/configuration/server_errors.html

		onLoginFailed
			Description: 
				Login failed.

			Event-datas: 
				uint16: retcode
				bytes: serverdatas
					http://kbengine.org/docs/configuration/server_errors.html

		onLoginBaseappFailed
			Description: 
				Login baseapp failed.

			Event-datas: 
				uint16: retcode
					http://kbengine.org/docs/configuration/server_errors.html

		onReloginBaseappFailed
			Description: 
				Relogin baseapp failed.

			Event-datas: 
				uint16: retcode
					http://kbengine.org/docs/configuration/server_errors.html

		onReloginBaseappSuccessfully
			Description: 
				Relogin baseapp success.

			Event-datas: 
				No datas.
	
	Space events:
		addSpaceGeometryMapping
			Description: 
				The current space is specified by the geometry mapping.
				Popular said is to load the specified Map Resources.

			Event-datas: 
				string: resPath

		onSetSpaceData
			Description: 
				Server spaceData set data.

			Event-datas: 
				int32: spaceID
				string: key
				string value

		onDelSpaceData
			Description: 
				Server spaceData delete data.

			Event-datas: 
				int32: spaceID
				string: key

	Network events:
		onConnectionState
			Description: 
				Status of connection server.

			Event-datas: 
				bool: success or fail

		onDisconnected
			Description: 
				Status of connection server.

			Event-datas: 
				No datas.

	Download events:
		onStreamDataStarted
			Description: 
				Start downloading data.

			Event-datas: 
				uint16: resouce id
				uint32: data size
				string: description

		onStreamDataRecv
			Description: 
				Receive data.

			Event-datas: 
				uint16: resouce id
				bytes: datas

		onStreamDataCompleted
			Description: 
				The downloaded data is completed.

			Event-datas: 
				uint16: resouce id



KBE-Plugin fire-in events(Unity => KBE):
---------------------

	createAccount
			Description: 
				Create new account.

			Event-datas: 
				string: accountName
				string: password
				bytes: datas
					Datas by user defined.
					Data will be recorded into the KBE account database, you can access the datas through the script layer.
					If you use third-party account system, datas will be submitted to the third-party system.
				
	login
			Description: 
				Login to server.

			Event-datas: 
				string: accountName
				string: password
				bytes: datas
					Datas by user defined.
					Data will be recorded into the KBE account database, you can access the datas through the script layer.
					If you use third-party account system, datas will be submitted to the third-party system.

	logout
			Description: 
				Logout to baseapp, called when exiting the client.

			Event-datas: 
				No datas.

	reloginBaseapp
			Description: 
				Relogin to baseapp.

			Event-datas: 
				No datas.

	resetPassword
			Description: 
				Reset password.

			Event-datas: 
				string: accountName

	newPassword
			Description: 
				Request to set up a new password for the account.
				Note: account must be online

			Event-datas: 
				string: old_password
				string: new_password

	bindAccountEmail
			Description: 
				Request server binding account Email.
				Note: account must be online

			Event-datas: 
				string: emailAddress


