/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEAccount()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
		this.avatars = {};
		
		this.reqCreateAvatar(1, "kbengine");
		this.baseCall("reqAvatarList");
	}
}

KBEAccount.prototype = new KBEGameObject();

KBEAccount.prototype.onCreateAvatarResult = function(v)
{
}

KBEAccount.prototype.onReqAvatarList = function(v)
{
}

KBEAccount.prototype.reqCreateAvatar = function(roleType, name)
{
	this.baseCall("reqCreateAvatar", roleType, name);
}

KBEAccount.prototype.selectAvatarGame = function(dbid)
{
	this.baseCall("selectAvatarGame", dbid);
}
