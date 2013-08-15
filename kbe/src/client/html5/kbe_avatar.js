/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEAvatar()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
	}
	
	this.__init__();
}

KBEAvatar.prototype = new KBEGameObject();
