#pragma once

#include "Component.h"
#include "Camera.h"

#include "mat.h"
#include "wavefront.h"
#include "texture.h"
#include "program.h"
#include "uniforms.h"

using namespace std;

/*!
*  \brief Classe g�rant un maillage et sa texture associ�s � un GameObject.
*/
class MeshRenderer : public Component
{
private:
	Mesh mesh;
	GLuint texture;
	GLuint shaderProgram;
	Color color = Color(1,0,0,1);

public:
	MeshRenderer(){}

	/*!
	*  \brief fonction appel�e � la fermeture de l'application pour tous les composants.
	*/
	void OnDestroy()
	{
		release_program(shaderProgram);
	}

	/*------------- -------------*/
	void Draw(Camera* target)
	{
		// Setup shader program for draw
		glUseProgram(shaderProgram);
		Transform mvp = target->GetProjectionMatrix() * (target->GetViewMatrix() * gameObject->GetObjectToWorldMatrix());
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "mvpMatrix"), 1, GL_TRUE, mvp.buffer());
		glUniform4fv(glGetUniformLocation(shaderProgram, "color"), 1, &color.r);

		// Draw mesh
		GLuint vao = mesh.GetVAO();
		if (vao == 0)
			mesh.create_buffers();
		glBindVertexArray(mesh.GetVAO());
		glUseProgram(shaderProgram);
		glDrawElements(GL_TRIANGLES, (GLsizei)mesh.indices().size(), GL_UNSIGNED_INT, 0);
	}

	/*!
	*  \brief R�cup�re le maillage de ce GameObject (par r�f�rence).
	*  \return le maillage de ce GameObject (par r�f�rence).
	*/
	Mesh& GetMesh()
	{
		return mesh;
	}

	/*!
	*  \brief Assigne un maillage � ce GameObject.
	*  \param m le maillage � assigner.
	*/
	void SetMesh(Mesh m)
	{
		mesh = m;
	}

	void SetColor(Color c)
	{
		color = c;
	}

	/*!
	*  \brief R�cup�re la texture associ�e � ce GameObject.
	*  \return la texture associ�e � ce GameObject.
	*/
	GLuint GetTexture()
	{
		return texture;
	}

	/*!
	*  \brief R�cup�re le shader associ�e � ce GameObject.
	*  \return le shader associ�e � ce GameObject.
	*/
	GLuint GetShader()
	{
		return shaderProgram;
	}

	/*!
	*  \brief Assigne un maillage � ce GameObject � partir d'un fichier.
	*  \param filename le chemin du fichier contenant le maillage � charger.
	*/
	void LoadMesh(const char *filename)
	{
		mesh = read_mesh(filename);
	}

	/*!
	*  \brief Assigne une texture � ce GameObject � partir d'un fichier.
	*  \param filename le chemin du fichier contenant la texture � charger.
	*/
	void LoadTexture(const char *filename)
	{
		texture = read_texture(0, filename);
	}

	/*!
	*  \brief Assigne un shader � ce GameObject � partir d'un fichier.
	*  \param filename le chemin du fichier contenant le shader � charger.
	*/
	void LoadShader(const char *filename)
	{
		shaderProgram = read_program(filename);
	}
	/*------------- -------------*/

};
