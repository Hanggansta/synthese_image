#pragma once

#include "Component.h"
#include "window.h"
#include "mat.h"

#include "CImg.h"
#include "GameObject.h"
#include "MeshRenderer.h"

using namespace cimg_library;

/*!
*  \brief Classe g�rant un terrain g�n�r� proc�duralement � partir d'une carte d'�levation.
*/
class GameTerrain : public Component
{
private:
	MeshRenderer* renderer;
	float terrainSize = 50.0f;
	float whiteAltitude = 10.0f;
	int verticesPerLine = 100.0f;
	string heightmap;
	CImg<float> image;

public:
	/*------------- -------------*/

	/*!
	*  \brief Constructeur pour pr�ciser tous les param�tres de terrain en une fois.
	*  \param h le chemin vers la carte d'�levation.
	*  \param wA l'altitude souhait�e des zones blanches de la carte d'�levation.
	*  \param tS la taille du terrain.
	*  \param vP le nombre de vertices par ligne souhait� pour le mesh du terrain (nombre total = vP * vP).
	*/
	GameTerrain(string h, float wA, float tS, int vP)
	{
		whiteAltitude = wA;
		terrainSize = tS;
		verticesPerLine = vP;
		heightmap = h;
	}

	/*!
	*  \brief Fonction appell�e au lancement de l'application. Initialise le terrain avec
	*	les param�tres donn�s au constructeur.
	*/
	void OnStart()
	{
		renderer = this->gameObject->GetComponent<MeshRenderer>();
		image = CImg<float>(heightmap.c_str());
		GenerateTerrainMesh();
	}
	/*------------- -------------*/

	/*!
	*  \brief Fonction d'interpolation lin�aire entre deux valeurs.
	*  \param a la valeur de d�but.
	*  \param b la valeur de fin.
	*  \param f la quantit� de la valeur de fin � retourner (entre 0.0f et 1.0f).
	*  \return la valeur interpol�e.
	*/
	float Lerp(float a, float b, float f)
	{
		return (a * (1.0f - f)) + (b * f);
	}

	/*!
	*  \brief G�n�re le maillage du terrain � partir de la carte d'�l�vation et
	*	des param�tres fournis dans le constructeur. Assigne le mesh au MeshRenderer
	*	sur le m�me GameObject (doit �tre pr�sent).
	*/
	void GenerateTerrainMesh()
	{
		Mesh meshHF(GL_TRIANGLES);

		float texelX = 1.0f / ((float)image.width());
		float texelY = 1.0f / ((float)image.height());

		for (int i = 0; i < verticesPerLine; i++)
		{
			for (int j = 0; j < verticesPerLine; j++)
			{
				float u = j / ((float)verticesPerLine - 1);
				float v = i / ((float)verticesPerLine - 1);

				int anchorX = u * (image.width() - 1);
				int anchorY = v * (image.height() - 1);

				if (anchorX == image.width() - 1)
					anchorX--;

				if (anchorY == image.height() - 1)
					anchorY--;

				float a = image(anchorX, anchorY, 0, 0) / 255.0f;
				float b = image(anchorX, anchorY + 1, 0, 0) / 255.0f;
				float c = image(anchorX + 1, anchorY + 1, 0, 0) / 255.0f;
				float d = image(anchorX + 1, anchorY, 0, 0) / 255.0f;


				float anchorU = anchorX * texelX;
				float anchorV = anchorY * texelY;

				float localU = (u - anchorU) / texelX;
				float localV = (v - anchorV) / texelY;

				float abu = Lerp(a, b, localU);
				float dcu = Lerp(d, c, localU);

				float height = Lerp(dcu, abu, localV) * whiteAltitude;
				meshHF.texcoord(u, 1 - v);
				//meshHF.normal(0.0f, 1.0f, 0.0f);
				meshHF.vertex(u * terrainSize, height, v * terrainSize);
			}
		}

		// Set indexes
		int nbTris = 2 * ((verticesPerLine - 1) * (verticesPerLine - 1));
		int c = 0;
		int vertexArrayLength = verticesPerLine * verticesPerLine;
		while (c < vertexArrayLength - verticesPerLine - 1)
		{
			if (c == 0 || (((c + 1) % verticesPerLine != 0) && c <= vertexArrayLength - verticesPerLine))
			{
				// First triangle of this quad
				meshHF.triangle(c, c + verticesPerLine, c + verticesPerLine + 1);

				// Second triangle of this quad
				meshHF.triangle(c + verticesPerLine + 1, c + 1, c);
			}
			c++;
		}
		renderer->SetMesh(meshHF);
	}

	/*!
	*  \brief R�cup�re la taille de ce terrain (longueur d'un c�t� du carr�)
	*  \return la taille de ce terrain (longueur d'un c�t� du carr�).
	*/
	float GetTerrainSize()
	{
		return terrainSize;
	}

	/*!
	*  \brief R�cup�re l'altitude dans le rep�re Monde du terrain aux coordonn�es X,Z monde sp�cifi�es.
	*  \param x la coordonn�e X monde.
	*  \param z la coordonn�e Z monde.
	*  \return la coordonn�e Y monde.
	*/
	float GetWorldAltitudeAt(float x, float z)
	{
		Vector position = this->gameObject->GetPosition();
		float u = (x - position.x) / terrainSize;
		float v = (z - position.z) / terrainSize;

		if (x != x || z != z || u < 0.0f || u > 1.0f || v < 0.0f || v > 1.0f)
			return -9999999;

		float texelX = 1.0f / ((float)image.width());
		float texelY = 1.0f / ((float)image.height());

		int anchorX = u * (image.width() - 1);
		int anchorY = v * (image.height() - 1);

		if (anchorX == image.width() - 1)
			anchorX--;

		if (anchorY == image.height() - 1)
			anchorY--;

		float a = image(anchorX, anchorY, 0, 0) / 255.0f;
		float b = image(anchorX, anchorY + 1, 0, 0) / 255.0f;
		float c = image(anchorX + 1, anchorY + 1, 0, 0) / 255.0f;
		float d = image(anchorX + 1, anchorY, 0, 0) / 255.0f;


		float anchorU = anchorX * texelX;
		float anchorV = anchorY * texelY;

		float localU = (u - anchorU) / texelX;
		float localV = (v - anchorV) / texelY;

		float abu = Lerp(a, b, localU);
		float dcu = Lerp(d, c, localU);

		float height = Lerp(dcu, abu, localV) * whiteAltitude;
		return height + position.y;
	}
};
