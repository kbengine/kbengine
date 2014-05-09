/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEAccount()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
		this.avatars = {};

		this.baseCall("reqAvatarList");
	}
}

KBEAccount.prototype = new KBEGameObject();

KBEAccount.prototype.onCreateAvatarResult = function(retcode, info)
{
	if(retcode == 0)
	{
		this.avatars[info.dbid] = info;
		this.avatars.values.push(info);
		console.info("KBEAccount::onCreateAvatarResult: name=" + info.name);
	}
	
	console.info("KBEAccount::onCreateAvatarResult: avatarsize=" + this.avatars.values.length);
}

KBEAccount.prototype.onReqAvatarList = function(infos)
{
	this.avatars = infos;
	console.info("KBEAccount::onReqAvatarList: avatarsize=" + this.avatars.values.length);
	for(var i=0; i< this.avatars.values.length; i++)
	{
		console.info("KBEAccount::onReqAvatarList: name" + i + "=" + this.avatars.values[i].name);
	}
}

KBEAccount.prototype.reqCreateAvatar = function(roleType, name)
{
	this.baseCall("reqCreateAvatar", roleType, name);
}

KBEAccount.prototype.selectAvatarGame = function(dbid)
{
	this.baseCall("selectAvatarGame", dbid);
}
