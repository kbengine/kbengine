/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEAccount()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
		this.avatars = {};
	}
	
	this.__init__();
}

KBEAccount.prototype = new KBEGameObject();

