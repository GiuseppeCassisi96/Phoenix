#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec2 inTexCoord;
layout(location = 3) in vec3 inOffsetPos;
layout(location = 4) in float inOffsetScale;
layout(location = 5) in ivec2 inBoneID;
layout(location = 6) in vec2 inWeight;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec2 fragTexCoord;

layout(binding = 0) uniform UniformBufferObject {
    mat4 model;
    mat4 view;
    mat4 proj;
} ubo;

const int MAX_BONES = 100;
const int MAX_BONE_PER_VERTEX = 2;

layout(binding = 2) uniform animBufferObk 
{
    mat4 finalBonesMatrices[MAX_BONES];

} anim;

void main() {

    vec4 totalPosition = vec4(0.0);
    for(int i = 0 ; i < MAX_BONE_PER_VERTEX ; i++)
    {
        if(inBoneID[i] == -1) 
        {
            totalPosition = vec4((inPosition * inOffsetScale) + inOffsetPos,1.0);
            break;
        }
            
        if(inBoneID[i] >=MAX_BONES) 
        {
            totalPosition = vec4((inPosition * inOffsetScale) + inOffsetPos,1.0);
            break;
        }
        vec4 localPosition = anim.finalBonesMatrices[inBoneID[i]] * 
        vec4(inPosition, 1.0);
        totalPosition += ((localPosition * inOffsetScale) + vec4(inOffsetPos, 1.0)) * inWeight[i];
    }

    gl_Position = ubo.proj * ubo.view * ubo.model * totalPosition;
    fragColor = inColor;
    fragTexCoord = inTexCoord;
}