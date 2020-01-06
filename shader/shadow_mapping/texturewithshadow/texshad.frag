#version 330 core

out vec4 FragColor;

in VS_OUT {
  vec3 FragPos;
  vec3 Normal;
  vec2 TexCoords;
  vec4 FragPosLightSpace;
} fs_in;

struct Material {
  sampler2D texture_specular1;
  sampler2D texture_specular2;
  sampler2D texture_diffuse1;
  sampler2D texture_diffuse2;
  sampler2D texture_normalMap1;
  sampler2D texture_normalMap2;
};
uniform Material material;

uniform sampler2D shadowMap;
// uniform sampler2DShadow shadowMap;

uniform vec3 lightPos;
uniform vec3 viewPos;

float shadow_calculation(vec4 fragPosLightSpace, float dot_normal_lightDir) {
  // prespective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // if outside of shadow-map
  if (projCoords.z > 1.0)
    return 0.0;
  // [-1, 1] -> [0, 1]
  projCoords = projCoords * 0.5 + 0.5;
  // closest depth value from the light's prespective
  float closestDepth = texture(shadowMap, projCoords.xy).r;
  // float closestDepth = shadow2D(shadowMap, vec3(projCoords.xy, 0.0)).r;
  // current depth of the fragment from light's prespective
  float currentDepth = projCoords.z;
  float shadow_bias = max(0.05 * (1.0 - dot_normal_lightDir), 0.005);
  // am I in the shadow region?
  return (currentDepth > closestDepth + shadow_bias) ? 1.0 : 0.0;
}

void main() {
  vec3 normal = normalize(fs_in.Normal);
  vec3 diffuseColor = texture(material.texture_diffuse1, fs_in.TexCoords).rgb;
  float specularIntensity = texture(material.texture_specular1, fs_in.TexCoords).r;
  vec3 lightColor = vec3(1.0);
  
  // ambient
  vec3 ambient = 0.15 * diffuseColor;

  //diffuse
  vec3 lightDir = normalize(lightPos - fs_in.FragPos);
  float dot_normal_lightDir = dot(normal, lightDir);
  float diff = max(dot_normal_lightDir, 0.0);
  vec3 diffuse = diff * lightColor;
  
  // specular
  vec3 viewDir = normalize(lightPos - fs_in.FragPos);
  vec3 halfwayDir = normalize(lightDir + viewDir);
  float spec = specularIntensity * pow(max(dot(normal, halfwayDir), 0.0), 64);
  vec3 specular = spec * lightColor;

  float shadow = shadow_calculation(fs_in.FragPosLightSpace, dot_normal_lightDir);
  vec3 lighting = (ambient + (1.0 - shadow) * (diffuse + specular)) * diffuseColor;
  FragColor = vec4(lighting, 1.0);
}
