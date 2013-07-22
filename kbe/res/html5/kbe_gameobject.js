/*-----------------------------------------------------------------------------------------
												entity
-----------------------------------------------------------------------------------------*/
function KBEGameObject()
{	
	KBEGameObject.prototype.inWorld = false;
	
	KBEGameObject.prototype.__init__ = function()
	{
	}
	
	KBEGameObject.prototype.enterWorld = function()
	{
		console.info(this.classtype + '::enterWorld: ' + this.id); 
		this.inWorld = true;
	}
	
	KBEGameObject.prototype.leaveWorld = function()
	{
		console.info(this.classtype + '::leaveWorld: ' + this.id); 
		this.inWorld = false;
	}
}

KBEGameObject.prototype = new KBEENTITY();

