Header
{
	Name: Diffuse
	Version: 330
	Lighting: On
		LightingModel: Lambert
		Specular: Off
		Normalmap: Off
	GammaCorrection: Off
}

Vertex
{
	out vec2 vTexCoord;
	
	void PreVertex() {}
	
	void PostVertex()
	{
		vTexCoord = inTexCoord;
	}
}

Geometry
{

}

Fragment
{
	in vec2 vTexCoord;
	uniform sampler2D uSampler;

	void PreFragment() {}
	void PostFragment()
	{
		fragColor.rgb *= texture2D(uSampler, vTexCoord.xy).rgb;
	}
}
 