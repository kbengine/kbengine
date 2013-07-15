/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEGate()
{
	this.__init__ = function()
	{
		KBEGameObject.prototype.__init__.call(this);
	}
	
	this.__init__();
}

KBEGate.prototype = new KBEGameObject();
