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
    vec3 f_specular =  Cs * pow(clamp (dot(V,r_lx), 0,1), gamma);
    return f_specular;
}

//compute the color
vec3 computeSpotColor(float beta, float g, vec3 pos, vec3 lightPos, vec3 lightColor, vec3 lightDir, float cos, float sin){
    vec3 fact1 = pow(beta/length(pos - lightPos), g) * lightColor;
    float fact2 = clamp((dot(normalize(lightPos - pos), lightDir) - cos) / (sin - cos),
                        0,
                        1);
    vec3 col = fact1 * fact2;
    return col;
}

vec3 Toon_Diffuse_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, vec3 Cd, float thr) {
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

vec3 Toon_Specular_BRDF(vec3 L, vec3 N, vec3 V, vec3 C, float thr)  {
    vec3 r = 2* N * dot(L, N) - L;
    vec3 f_specular = C;
    if (dot(V, r) < thr){
        f_specular = vec3(0.0f);
    }
    return f_specular;
}





void main() {
    vec3 Norm = normalize(fragNorm);
    vec3 EyeDir = normalize(gubo.eyePos.xyz - fragPos);
    float gamma = 50.0f;
    vec3 TextureCol = texture(texSampler, fragTexCoord).rgb;
    vec3 GreyCol = vec3(min(min(TextureCol.r, TextureCol.g),TextureCol.b));
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
    
    //Lambert diffuse
    //vec3 lightDiff = computeLambertDiff(Norm, fragPos, gubo.lightPos ,DifCol);
    vec3 Diff1 = gubo.selector.y *computeLambertDiff(Norm, fragPos, pos1 ,DifCol)  ;
    vec3 Diff2 = computeLambertDiff(Norm, fragPos, pos2 ,DifCol);
    vec3 Diff3 = computeLambertDiff(Norm, fragPos, pos3 ,DifCol);
    vec3 Diff4 = computeLambertDiff(Norm, fragPos, pos4 ,DifCol);
    vec3 Diff5 = computeLambertDiff(Norm, fragPos, pos5 ,DifCol);
    vec3 Diff6 = computeLambertDiff(Norm, fragPos, pos6 ,DifCol);
    vec3 Diff7 = computeLambertDiff(Norm, fragPos, pos7 ,DifCol);
    vec3 Diff8 = computeLambertDiff(Norm, fragPos, pos8 ,DifCol);
    
    //Phong specular
    //vec3 lightSpec = computePhongSpec(Norm, fragPos, gubo.lightPos, SpecCol, EyeDir, gamma);
    vec3 Spec1 = computePhongSpec(Norm, fragPos, pos1, SpecCol, EyeDir, gamma);
    vec3 Spec2 = computePhongSpec(Norm, fragPos, pos2, SpecCol, EyeDir, gamma);
    vec3 Spec3 = computePhongSpec(Norm, fragPos, pos3, SpecCol, EyeDir, gamma);
    vec3 Spec4 = computePhongSpec(Norm, fragPos, pos4, SpecCol, EyeDir, gamma);
    vec3 Spec5 = computePhongSpec(Norm, fragPos, pos5, SpecCol, EyeDir, gamma);
    vec3 Spec6 = computePhongSpec(Norm, fragPos, pos6, SpecCol, EyeDir, gamma);
    vec3 Spec7 = computePhongSpec(Norm, fragPos, pos7, SpecCol, EyeDir, gamma);
    vec3 Spec8 = computePhongSpec(Norm, fragPos, pos8, SpecCol, EyeDir, gamma);
    
    
    //Spot light color
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
    
    vec3 computed_col = Col1*(Diff1+Spec1) + Col2*(Diff2+Spec2) + Col3*(Diff3+Spec3) + Col4*(Diff4+Spec4) + Col5*(Diff5+Spec5) + Col6*(Diff6+Spec6) + Col7*(Diff7+Spec7) + Col8*(Diff8+Spec8) + ambient ;
    outColor = vec4(computed_col, 1.0);
    
    /*
    gl_Position = gubo.proj * gubo.view * ubo.model * vec4(pos, 1.0);
    fragViewDir  = (gubo.view[3]).xyz - (ubo.model * vec4(pos,  1.0)).xyz;
    fragNorm     = (ubo.model * vec4(norm, 0.0)).xyz;
    fragTexCoord = texCoord;
     */
}
