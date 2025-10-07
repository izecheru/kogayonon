#version 460 core

layout (location = 0) out uint outEntityID;

flat in uint v_entityId;

void main()
{
    outEntityID = 100;
}