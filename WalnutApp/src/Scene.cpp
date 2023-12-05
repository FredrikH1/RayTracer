#include "Scene.h"

namespace scene
{
	void SaveScene(const Scene& scene, const char* Directory)
	{
		FILE* fd = std::fopen(Directory, "wb");
		void* buffer = malloc(sizeof(Scene));
		memcpy(buffer, &scene, sizeof(Scene));
		std::fwrite(buffer, sizeof(Scene), 1, fd);

		if (scene.Spheres.size())
		{
			buffer = realloc(buffer, sizeof(Sphere) * scene.Spheres.size());
			memcpy(buffer, scene.Spheres.data(), sizeof(Sphere) * scene.Spheres.size());
			std::fwrite(buffer, sizeof(Sphere), scene.Spheres.size(), fd);
		}

		if (scene.Triangles.size())
		{
			buffer = realloc(buffer, sizeof(Triangle) * scene.Triangles.size());
			memcpy(buffer, scene.Triangles.data(), sizeof(Triangle) * scene.Triangles.size());
			std::fwrite(buffer, sizeof(Triangle), scene.Triangles.size(), fd);
		}

		if (scene.Materials.size())
		{
			buffer = realloc(buffer, sizeof(Material) * scene.Materials.size());
			memcpy(buffer, scene.Materials.data(), sizeof(Material) * scene.Materials.size());
			std::fwrite(buffer, sizeof(Material), scene.Materials.size(), fd);
		}
		

		std::fclose(fd);
		free(buffer);
	}

	bool LoadScene(Scene& scene, const char* Directory)
	{
		FILE* fd = std::fopen(Directory, "rb");
		if (fd) {
			scene.Spheres.clear();
			void* buffer = malloc(sizeof(Scene));
			std::fread(buffer, sizeof(Scene), 1, fd);
			scene.LightDir = ((Scene*)buffer)->LightDir;
			scene.BackgroundColor = ((Scene*)buffer)->BackgroundColor;
			int nbrSpheres = ((Scene*)buffer)->Spheres.size();
			int nbrMaterials = ((Scene*)buffer)->Materials.size();
			int nbrTriangles = ((Scene*)buffer)->Triangles.size();


			if (nbrSpheres)
			{
				buffer = realloc(buffer, sizeof(Sphere) * nbrSpheres);
				std::fread(buffer, sizeof(Sphere), nbrSpheres, fd);
				scene.Spheres.assign((Sphere*)buffer, (Sphere*)buffer + nbrSpheres);
			}

			if (nbrTriangles)
			{
				buffer = realloc(buffer, sizeof(Triangle) * nbrTriangles);
				std::fread(buffer, sizeof(Triangle), nbrTriangles, fd);
				scene.Triangles.assign((Triangle*)buffer, (Triangle*)buffer + nbrTriangles);
			}

			if (nbrMaterials)
			{
				buffer = realloc(buffer, sizeof(Material) * nbrMaterials);
				std::fread(buffer, sizeof(Material), nbrMaterials, fd);
				scene.Materials.assign((Material*)buffer, (Material*)buffer + nbrMaterials);
				std::fclose(fd);
			}
			free(buffer);
			return true;
			
		}
		return false;
	}
}