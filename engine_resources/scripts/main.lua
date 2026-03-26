local main = {}

mainRegistry = Registry()
eventDispatcher = EventDispatcher()

luaEvHandler = LuaEventHandler(function(event)
	local result = tonumber(event.data and event.data.x) or 0
	print(result * result)
end)

function main:assertions()
	print("Script loaded")
	local entity = Entity(mainRegistry, "Test")
	assert(entity:getEntityId() == 0, "entity was not created")
	print("Entity was created")
	assert(entity:hasComponent(IdentifierComponent) == true, "entity does not have an IdentifierComponent")
	print("Entity has IdentifierComponent")
	entity:addComponent(IndexComponent(1))
	assert(entity:hasComponent(IndexComponent) == true, "entity does not have an IndexComponent")
	print("IndexComponent was added")
	entity:removeComponent(IndexComponent)
	assert(entity:hasComponent(IndexComponent) == false, "entity could not remove IndexComponent")
	print("IndexComponent removed")

	eventDispatcher:addHandler(LuaEvent, luaEvHandler)
	assert(eventDispatcher:hasHandler(LuaEvent) == true, "could not add any handlers")
	eventDispatcher:dispatchEvent(LuaEvent({ x = 2 }))
	eventDispatcher:removeHandler(LuaEvent, luaEvHandler)
	eventDispatcher:dispatchEvent(LuaEvent({ x = 2 }))
	assert(eventDispatcher:hasHandler(LuaEvent) == false, "could not remove handlers")
	print("handler was removed")
end

main:assertions()