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

KBEAccount.prototype.onCreateAvatarResult = function(v)
{
}

KBEAccount.prototype.onReqAvatarList = function(v)
{
	this.avatars = v;
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
