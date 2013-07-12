/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEGameObject()
{	
	KBEGameObject.prototype.__init__ = function()
	{
	}
	
	KBEGameObject.prototype.enterWorld = function()
	{
		console.info(this.classtype + '::enterWorld: ' + this.id) ; 
	}
	
	KBEGameObject.prototype.leaveWorld = function()
	{
		console.info(this.classtype + '::leaveWorld: ' + this.id) ; 
	}
}

KBEGameObject.prototype = new KBEENTITY();

