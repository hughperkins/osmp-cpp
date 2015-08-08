

#include "WorldStorage.h"
#include "collisionandphysicsdllloader.h"
#include "texturemanager.h"
#include "TerrainInfoCache.h"

CollisionAndPhysicsDllLoaderClass CP;
mvWorldStorage World;

//TextureInfoCache textureinfocache;
//TerrainCacheClass TerrainCache;

void main()
{
	CP.LoadDll( "collisionandphysicsdllprot.dll" );
	
	CP.HandleCollisionsAndPhysics( World );
	
	CP.UnloadDll();
}


