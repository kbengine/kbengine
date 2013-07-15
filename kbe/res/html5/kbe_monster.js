/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEMonster()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
	}
	
	this.__init__();
}

KBEMonster.prototype = new KBEGameObject();
