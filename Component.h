#pragma once

class GameObject;

/*!
*  \brief Classe de base dont h�rite tous les composants.
*/
class Component
{
protected:
	/*!
	*  \brief le GameObject auquel ce composant est assign�.
	*/
	GameObject* gameObject = nullptr;

public:
	/*!
	*  \brief fonction appel�e au lancement de l'application pour tous les composants.
	*/
	virtual void OnStart() {}

	/*!
	*  \brief fonction appel�e � la fermeture de l'application pour tous les composants.
	*/
	virtual void OnDestroy() {}

	/*!
	*  \brief fonction appel�e � chaque frame de l'application pour tous les composants.
	*  \param dt le temps en millisecondes �coul� depuis la derni�re frame.
	*/
	virtual void Update(float dt) {}

	/*!
	*  \brief Assigne le GameObject auquel ce composant appartient.
	*  \param gameobject le GameObject auquel ce composant appartient.
	*/
	void SetGameObject(GameObject* gameobject)
	{
		this->gameObject = gameobject;
	}

	/*!
	*  \brief R�cup�re le GameObject auquel ce composant appartient.
	*  \return le GameObject auquel ce composant appartient.
	*/
	GameObject* GetGameObject()
	{
		return this->gameObject;
	}
};
