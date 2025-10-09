#version 460 core

layout (location = 0) out int outEntityID;

flat in int v_entityId;

void main()
{
    outEntityID = v_entityId;
}