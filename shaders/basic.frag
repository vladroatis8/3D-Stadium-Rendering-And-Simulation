#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;
// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;

uniform bool dirLightEnabled; 
uniform vec3 pointLightPos; 
uniform vec3 pointLightColor;
uniform bool pointLightEnabled;
uniform float fogDensity;

//components
vec3 ambient;
float ambientStrength = 0.2f;
vec3 diffuse;
vec3 specular;
float specularStrength = 0.5f;

vec3 pambient;
vec3 pdiffuse;
vec3 pspecular;
float c=1.0f;
float l=7.0f;
float q=8.0f;


void computeDirLight()
{
    if (dirLightEnabled) {
        vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
        vec3 normalEye = normalize(normalMatrix * fNormal);

        vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

        vec3 viewDir = normalize(-fPosEye.xyz);

        ambient = ambientStrength * lightColor;
        diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

        vec3 reflectDir = reflect(-lightDirN, normalEye);
        float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
        specular = specularStrength * specCoeff * lightColor;
    } else {
        ambient = vec3(0.0f);
        diffuse = vec3(0.0f);
        specular = vec3(0.0f);
    }
}


void computePointLight(){
if(pointLightEnabled)
{
 vec3 normalEye = normalize(normalMatrix * fNormal);	
    vec3 lightDirN = normalize(pointLightPos - fPosition.xyz);    
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    //compute distance to light

    float dist = length(pointLightPos - fPosition.xyz);
    vec3 viewDir = normalize (-fPosEye.xyz);

    //compute attenuation
    float aten = 1.0f / (c + l* dist + q * dist*dist);

    
    
    //compute ambient light
    pambient = aten * pointLightColor;
    //compute diffuse light
    pdiffuse = aten * max(dot(normalEye, lightDirN), 0.0f) * pointLightColor;
	vec3 reflection = reflect(-lightDirN, normalEye);
	float specCoeff = pow(max(dot(viewDir, reflection), 0.0f), 0.32f);
    pspecular = aten * specularStrength * specCoeff * pointLightColor;
}else{
        pambient = vec3(0.0f);
        pdiffuse = vec3(0.0f);
        pspecular = vec3(0.0f);
    }
}

float computeFog()
{
 vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
 float fragmentDistance = length(fPosEye);
 float fogFactor = exp(-pow(fragmentDistance * fogDensity, 2));

 return clamp(fogFactor, 0.0f, 1.0f);
}


void main() 
{
    computeDirLight();
    computePointLight();

    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
    vec3 viewDir = normalize(-fPosEye.xyz);

    vec3 color = min((ambient+pambient + pdiffuse + diffuse) * texture(diffuseTexture, fTexCoords).rgb +
                        (specular+pspecular) * texture(specularTexture, fTexCoords).rgb, 1.0f);

    float fogFactor = computeFog();
    vec4 fogColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
    vec4 colorw=vec4(color, 1.0f);
    colorw=mix(fogColor, colorw, fogFactor);
    fColor = colorw;
}
