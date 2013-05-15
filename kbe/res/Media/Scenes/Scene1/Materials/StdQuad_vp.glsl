varying vec2 uv;

void main()                    
{
	gl_Position = ftransform();
	
	vec2 inPos = sign(gl_Vertex.xy);
	
	uv = (vec2(inPos.x, -inPos.y) + 1.0)/2.0;
}