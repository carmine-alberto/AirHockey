#version 450

layout(set = 0, binding = 0) uniform globalUniformBufferObject {
    mat4 view;
    mat4 proj;
    vec3 lightColor;
    vec3 lightPos;
    vec3 ambColor;
    vec4 coneInOutDecayExp;
    vec3 spotPosition1;
    vec3 spotPosition2;
    vec3 spotDirection;
    vec3 eyePos;
    vec2 selector;
} gubo;

layout(set = 1, binding = 0) uniform UniformBufferObject {
    mat4 model;
} ubo;


layout(set=1, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec3 fragPos;
layout(location = 1) in vec3 fragNorm;
layout(location = 2) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;



//computes diffuse component of light with Lambert
vec3 computeLambertDiff(vec3 N, vec3 pos, vec3 lightPos, vec3 Cd){
    
    vec3 direction = normalize(lightPos - pos);
    float cos_a = dot(N, direction);
    vec3 f_diffuse = Cd * clamp(cos_a, 0, 1);
    return f_diffuse;
}

//compute specular component of light with phong
vec3 computePhongSpec(vec3 N, vec3 pos, vec3 lightPos, vec3 Cs, vec3 V, float gamma){
    
    vec3 direction = normalize(lightPos - pos);
    vec3 r_lx = -reflect(direction, N);
    vec3 f_specular =  Cs * pow(clamp(dot(V,r_lx), 0,1), gamma);
    return f_specular;
}

//compute the color
vec3 computeSpotColor(float beta, float g, vec3 pos, vec3 lightPos, vec3 lightColor, vec3 lightDir, float cos_out, float cos_in){
    vec3 fact1 = pow(g/length(pos - lightPos), beta) * lightColor;
    float fact2 = clamp((dot(normalize(lightPos - pos), lightDir) - cos_out) / (cos_in - cos_out),
                        0,
                        1);
    vec3 col = fact1 * fact2;
    return col;
}

vec3 Toon_Diffuse_BRDF(vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 C, vec3 Cd, float thr) {
    
	vec3 L = normalize(lightPos - pos);
	float LN = dot(L, N);
    vec3 f_specular = C;// Light areas
    if (LN < 0){
        return vec3(0,0,0); // Opposite area
    }
    if (LN < thr){
        return Cd; // Dark areas
    }
    return f_specular;
}

vec3 Toon_Specular_BRDF(vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 C, float thr)  {
	vec3 L = normalize(lightPos - pos);
    vec3 r = 2* N * dot(L, N) - L;
    vec3 f_specular = C;
    if (dot(V, r) < thr){
        f_specular = vec3(0.0f);
    }
    return f_specular;
}



void main() {
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos - fragPos);
    float gamma = 50.0f;
    vec3 TextureCol = texture(texSampler, fragTexCoord).rgb;
    vec3 GreyCol = 2.0f / 3.0f * vec3(max(max(TextureCol.r, TextureCol.g),TextureCol.b));
    vec3 DifCol= gubo.selector.x * TextureCol + (1-gubo.selector.x) * GreyCol;
    vec3 SpecCol = vec3(1.0f, 1.0f, 1.0f);
    
    
    vec3 pos1= gubo.spotPosition1 + vec3(-0.8f, 1.0f, 0.0f);
    vec3 pos2= gubo.spotPosition1 + vec3(-0.25f, 1.0f, 0.0f);
    vec3 pos3= gubo.spotPosition1 + vec3(0.25f, 1.0f, 0.0f);
    vec3 pos4= gubo.spotPosition1 + vec3(0.8f, 1.0f, 0.0f);
    vec3 pos5= gubo.spotPosition2 + vec3(-0.8f, 1.0f, 0.0f);
    vec3 pos6= gubo.spotPosition2 + vec3(-0.25f, 1.0f, 0.0f);
    vec3 pos7= gubo.spotPosition2 + vec3(0.25f, 1.0f, 0.0f);
    vec3 pos8= gubo.spotPosition2 + vec3(0.8f, 1.0f, 0.0f);
    
    //Lambert diffuse OR Toon Diffuse
	//Toon: vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 C, vec3 Cd, float thr
    //vec3 lightDiff = computeLambertDiff(Norm, fragPos, gubo.lightPos ,DifCol);
    vec3 Diff1 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos1 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos1, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff2 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos2 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos2, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff3 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos3 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos3, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff4 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos4 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos4, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff5 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos5 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos5, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff6 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos6 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos6, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff7 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos7 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos7, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    vec3 Diff8 = gubo.selector.y * computeLambertDiff(Norm, fragPos, pos8 ,DifCol) + (1.0f - gubo.selector.y) * Toon_Diffuse_BRDF(pos8, fragPos, Norm, EyeDir, DifCol, DifCol*0.8f, 0.85);
    
    //Phong specular
    //vec3 lightSpec = computePhongSpec(Norm, fragPos, gubo.lightPos, SpecCol, EyeDir, gamma); ///Toon: Toon_Specular_BRDF(vec3 lightPos, vec3 pos, vec3 N, vec3 V, vec3 C, float thr)    
    vec3 Spec1 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos1, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos1, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec2 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos2, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos2, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec3 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos3, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos3, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec4 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos4, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos4, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec5 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos5, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos5, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec6 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos6, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos6, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec7 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos7, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos7, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    vec3 Spec8 = gubo.selector.y * computePhongSpec(Norm, fragPos, pos8, SpecCol, EyeDir, gamma) + (1.0f - gubo.selector.y) * Toon_Specular_BRDF(pos8, fragPos, Norm, EyeDir, vec3(1.0f), 0.93);
    
    
    //Spot light color
	//IN
	//float beta, float g, vec3 pos, vec3 lightPos, vec3 lightColor, vec3 lightDir, float cos_out, float cos_in
    //vec3 lightCol = pow(gubo.coneInOutDecayExp.z/length(fragPos - gubo.lightPos), gubo.coneInOutDecayExp.w) * gubo.lightColor;
    vec3 Col1 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos1, gubo.lightColor, gubo.spotDirection,gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col2 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos2, gubo.lightColor, gubo.spotDirection, gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col3 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos3, gubo.lightColor, gubo.spotDirection,gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col4 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos4, gubo.lightColor, gubo.spotDirection, gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col5 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos5, gubo.lightColor, gubo.spotDirection,gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col6 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos6, gubo.lightColor, gubo.spotDirection, gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col7 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos7, gubo.lightColor, gubo.spotDirection,gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    vec3 Col8 = computeSpotColor(gubo.coneInOutDecayExp.w, gubo.coneInOutDecayExp.z, fragPos, pos8, gubo.lightColor, gubo.spotDirection, gubo.coneInOutDecayExp.x, gubo.coneInOutDecayExp.y);
    
    
    //Ambient
    vec3 ambient = gubo.ambColor*DifCol;
    
    vec3 computed_col = clamp(Col1*(Diff1+Spec1) + Col2*(Diff2+Spec2) + Col3*(Diff3+Spec3) + Col4*(Diff4+Spec4) + Col5*(Diff5+Spec5) + Col6*(Diff6+Spec6) + Col7*(Diff7+Spec7) + Col8*(Diff8+Spec8) + ambient, 0, 1);
    outColor = vec4(computed_col, 1.0);
    
}
